#include "function.h"
#include "projdefs.h"
#include "step_28byj48.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <stdio.h>
#include "at24cxx.h"
#include "ds18b20.h"
#include "tim.h"
#include <stdlib.h> // 提供 abs() 绝对值函数

//雨水检测设备默认为 1为无雨 0为有雨 这里反转一下
// #define rain_check (!!(GPIOB->IDR&GPIO_PIN_1))
#define rain_check (!(GPIOB->IDR & GPIO_PIN_1))

node2_total_state node2_state; //定义总状态对象
node2_threshold threshold_one;  //定义阈值对象

extern eeprom_t       eeprom_one; // 你的 EEPROM 对象
extern Stepper_28BYJ48_t  g_motor;  // 你的步进电机对象

void node2_device_init()
{
    /* 1. 硬件外设启动 */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0); // 初始风扇停转
    Stepper_PowerOff(&g_motor);                      // 电机释放锁死，防止发热

    //读eeprom 校验
    eeprom_read(&eeprom_one, 0, (uint8_t *)&threshold_one, sizeof(node2_threshold));
    
    // 如果魔数不对，说明是新芯片或数据损坏，写入出厂默认配置
    if (threshold_one.magic_num != NODE2_MAGIC_NUM)
    {
        threshold_one.magic_num        = NODE2_MAGIC_NUM;
        threshold_one.temp_lev1_thresh = 30; // 30℃
        threshold_one.temp_lev2_thresh = 40; // 40℃
        threshold_one.temp_lev3_thresh = 50; // 50℃
        
        threshold_one.timeout_lev2_s   = 60; // 60秒
        threshold_one.timeout_lev3_s   = 30; // 30秒

        threshold_one.hys_val          = 2;  // 2℃ 回差
        
        // 固化到 AT24C02
        eeprom_write(&eeprom_one, 0, (const uint8_t *)&threshold_one, sizeof(node2_threshold));
    }

    /* 3. 运行状态赋予出厂安全值 */
    node2_state.current_temp           = 250;           // 默认假定 25.0℃，等待传感器首次刷新
    node2_state.rain_state             = rain_check;
    node2_state.work_mode              = node2_auto;    // 默认自动模式
    node2_state.temp_level             = normal;        // 默认正常工况
    node2_state.louver_target_percent  = 0;             // 百叶窗目标：全关
    node2_state.louver_current_percent = 0;             // 百叶窗当前：全关
    node2_state.fan_pwm_precent        = 0;             // 风扇：0%

}



void stepper_control()
{
   uint16_t  step = (uint16_t )Stepper_AngleToSteps(90);

    if(rain_check)
    {
        Stepper_PowerOff(&g_motor);
    }
    else 
    {
        for(uint16_t k=0;k<step;k++)
        {
            Stepper_Step(&g_motor,STEPPER_DIR_CW);
             HAL_Delay(4);
          
        }
         Stepper_PowerOff(&g_motor);//转完停
    }
}





/**
 * @brief node2 温度状态更新函数
 * 
 * @param dev   传入状态更新结构体
 */
void node2_state_update(node2_total_state * dev)
{
    if(dev->work_mode == node2_manual)return;   //手动模式下不执行自动逻辑

    // 实时更新雨水状态
    node2_state.rain_state = rain_check;

    uint16_t cur_t = node2_state.current_temp; // 当前温度 (x10)
    
    // 统一换算为 x10 格式
    uint16_t t_l1 = threshold_one.temp_lev1_thresh * 10; // 如 300 (30.0℃)
    uint16_t t_l2 = threshold_one.temp_lev2_thresh * 10; // 如 400 (40.0℃)
    uint16_t t_l3 = threshold_one.temp_lev3_thresh * 10; // 如 500 (50.0℃)
    uint16_t hys  = threshold_one.hys_val * 10;          // 如 20  (2.0℃)


    // 系统运行时间戳   
    uint32_t now_tick = xTaskGetTickCount();    //注意记录的是tick不绝对是1ms

    // 记录进入某个状态的时间戳
    static uint32_t state_enter_tick = 0;
    temp_state next_state = node2_state.temp_level;     //下一次状态  如果下面不发生改变那么依旧等于当前

    //第一个switch也不是100%就一定执行 会进入分支 具体状态是否更新和温度相关
    switch (node2_state.temp_level) //更具当前等级来
    {
        case normal :   //从normal开始 判定什么情况下升级到对应等级
            if(cur_t >= t_l1)       //如果当前温度大于阈值1 
            {
                next_state = temp_high_lev1;    //下次状态更新为 lev1
            }
        ;break;

        case temp_high_lev1 :
            if(cur_t >= t_l2)
            {
                next_state = temp_high_lev2;
            }
            else if(cur_t < (t_l1 - hys))   //减去回滞值
            {
                next_state = normal;
            }
        ;break;

        case temp_high_lev2 :
            // 升温到三级
            if (cur_t >= t_l3) {
                next_state = temp_high_lev3;
            }
            // 降温回一级（必须跌破回差）
            else if (cur_t < (t_l2 - hys)) {
                next_state = temp_high_lev1;
            }
        ;break;

        case temp_high_lev3 :               //3到异常不通过温度升高判定 
            // 降温回二级（必须跌破回差）
            if (cur_t < (t_l3 - hys)) {
                next_state = temp_high_lev2;
            }
        ;break;

        case temp_abnormal:                 //但是进入异常3级 通过降温回到2级
            if (cur_t < (t_l3 - hys)) next_state = temp_high_lev2;  
            break;
    }

    /* =========================================================
     * 第二部分：超时未降温强行升级（超时判定）
     * ========================================================= */
    // 只有在没有发生降级、仍保持在原状态时，才去数时间
    if (next_state == node2_state.temp_level)   
    {
        uint32_t stay_duration_ms = (now_tick - state_enter_tick) * portTICK_PERIOD_MS;

        // 1. 二级高温持续超时 -> 强升三级
        if (node2_state.temp_level == temp_high_lev2)
        {
            if (stay_duration_ms >= (threshold_one.timeout_lev2_s * 1000UL))
            {
                next_state = temp_high_lev3; // 超时强行升级
            }
        }
        // 2. 三级高温持续超时 -> 触发冷却异常
        else if (node2_state.temp_level == temp_high_lev3)
        {
            if (stay_duration_ms >= (threshold_one.timeout_lev3_s * 1000UL))
            {
                next_state = temp_abnormal;  // 标记异常
                // 发送 CAN 紧急帧（只在初次切入时发送一次）
              //  Node2_Send_Emergency_Frame();
            }
        }
    }
     /* =========================================================
     * 第三部分：状态转移与计时器刷新
     * ========================================================= */
    if (next_state != node2_state.temp_level)
    {
        // 核心细节：只要状态发生了改变（无论是升级还是降级），立刻重置计时基准！
        state_enter_tick = now_tick;
        node2_state.temp_level = next_state;
    }
}


// 第 1 维：天气索引
#define WEATHER_SUNNY   0   // 晴天（无雨）
#define WEATHER_RAINY   1   // 雨天（有雨）

// 第 3 维：执行器索引
#define ACT_LOUVER      0   // 百叶窗目标开度 %
#define ACT_FAN         1   // 风扇 PWM 占空比 %

/**
 * @brief 输出规则三维表 [天气 2 种][温度 5 级][执行器 2 个]
 */
static const uint8_t g_output_matrix[2][5][2] = {
    // =========================================================================
    // 【表 0：晴天 / 无雨工况表】
    // =========================================================================
    [WEATHER_SUNNY] = {
        /* normal        */ {0,   0},   // 正常：窗 0%,   扇 0%
        /* temp_high_lev1*/ {30,  0},   // 1级： 窗 30%,  扇 0%[cite: 6]
        /* temp_high_lev2*/ {100, 70},  // 2级： 窗 100%, 扇 70%[cite: 6]
        /* temp_high_lev3*/ {100, 100}, // 3级： 窗 100%, 扇 100%[cite: 6]
        /* temp_abnormal */ {100, 100}  // 异常：窗 100%, 扇 100%[cite: 6]
    },

    // =========================================================================
    // 【表 1：雨天 / 有雨工况表】（百叶窗一律关死为 0%，全靠风扇散热）[cite: 6]
    // =========================================================================
    [WEATHER_RAINY] = {
        /* normal        */ {0,   0},   // 正常：窗 0%,   扇 0%[cite: 6]
        /* temp_high_lev1*/ {0,   35},  // 1级： 窗 0%,   扇 35%[cite: 6]
        /* temp_high_lev2*/ {0,   100}, // 2级： 窗 0%,   扇 100%[cite: 6]
        /* temp_high_lev3*/ {0,   100}, // 3级： 窗 0%,   扇 100%[cite: 6]
        /* temp_abnormal */ {0,   100}  // 异常：窗 0%,   扇 100%[cite: 6]
    }
};

/**
 * @brief 根据当前天气和温度等级，计算并应用执行器输出
 * 
 * @details 该函数在自动模式下查表获取百叶窗和风扇的目标值，并在下雨时强制关闭百叶窗。
 * 
 */
void node2_apply_outputs(void)
{
    // 自动模式下查表
    if (node2_state.work_mode == node2_auto)
    {
        // 1. 确定第 1 维：今天下不下雨？
        uint8_t w_idx = node2_state.rain_state ? WEATHER_RAINY : WEATHER_SUNNY;
        
        // 2. 确定第 2 维：当前几级温度？
        uint8_t t_idx = (uint8_t)node2_state.temp_level;

        // 3. 从对应的表格里直接提走执行参数
        node2_state.louver_target_percent = g_output_matrix[w_idx][t_idx][ACT_LOUVER];
        node2_state.fan_pwm_precent       = g_output_matrix[w_idx][t_idx][ACT_FAN];
    }

    // 雨水最高安全逻辑防线：即使在手动模式下，下雨也绝对强制关窗[cite: 6]
    if (node2_state.rain_state)
    {
        node2_state.louver_target_percent = 0;
    }

    // // 硬件输出
     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, node2_state.fan_pwm_precent);
}


void get_temp(node2_total_state * dev,Ds18bxx_t * ds18b20_t)
{
    float current_temperature;
    static uint8_t fail_count=0;
    if(DS18B20_StartConversion(ds18b20_t))       //温度获取逻辑
    {
        vTaskDelay(pdMS_TO_TICKS(750)); 
        if (DS18B20_ReadTemp(ds18b20_t, &current_temperature))
        {
                fail_count=0;    //成功就清除
                dev->current_temp=(uint16_t)(current_temperature*10);   //数值扩大10倍  存入总状态里面去
                // 将 current_temperature 打包发送至 CAN 报文队列
                printf("Current temperature: %.1f°C\r\n", current_temperature);
        }
        else
        {
                fail_count++;
                printf("[Sensor Warning] Read scratchpad failed (count: %d)\r\n", fail_count);
        }
        
        if(fail_count>=3)
        {
            fail_count++;
            printf("[Sensor Error] Failed to read temperature after 3 attempts\r\n");
        }
    }
}

// 假设百叶窗从 0% (全关) 到 100% (全开) 对应转动 360 度
// 直接复用你驱动里的 Stepper_AngleToSteps 计算 360 度的总步数
#define LOUVER_FULL_OPEN_STEPS   ((uint16_t)Stepper_AngleToSteps(360))

/**
 * @brief 步进电机开机物理归零校准
 * @note  必须在 main() 函数中、FreeRTOS 调度器启动（osKernelStart）之前调用一次！
 */
void Stepper_Zero_Calibrate(void)
{
    // 强制往“关窗方向”反转 100% 行程的步数，把百叶窗强行顶到底部机械边缘贴死
    uint16_t full_steps = LOUVER_FULL_OPEN_STEPS;

    for (uint16_t i = 0; i < full_steps; i++)
    {
        Stepper_Step(&g_motor, STEPPER_DIR_CCW); // CCW 反转为关窗[cite: 4]
        HAL_Delay(3); // 此时 OS 还没启动，可以使用裸机死等延时
    }

    // 碰到底部贴死后，断电释放线圈，防止电机堵转发热[cite: 4]
    Stepper_PowerOff(&g_motor);

    // 软件状态与全局状态强行同步清零
    node2_state.louver_current_percent = 0;
}

/**
 * @brief  百叶窗开度执行函数（差值驱动）
 * @param  target_percent: 目标开度百分比 (0, 30, 70, 100)
 * @note   在 FreeRTOS 任务中周期调用，只在目标发生变化时才驱动电机
 */
void Stepper_Louver_Control(uint8_t target_percent)
{
    static uint8_t s_current_percent = 0  ;             //始终记住当前开度百分比，初始为0 

    if (target_percent > 100) target_percent = 100;
    if (target_percent == s_current_percent) return;            //达到目标开度，直接返回

    int16_t diff_percent = (int16_t)target_percent - (int16_t)s_current_percent;    //计算目标开度与当前开度的差值
    uint16_t steps_to_move = (abs(diff_percent) * LOUVER_FULL_OPEN_STEPS) / 100;    //计算需要移动的步数

    uint8_t direction = (diff_percent > 0) ? STEPPER_DIR_CW : STEPPER_DIR_CCW;  // 根据差值正负确定电机转动方向
    // 【调试打印】看走多少步
    printf("[Motor] Cur:%d%% -> Target:%d%%, Moving Steps:%d\r\n", 
           s_current_percent, target_percent, steps_to_move);

    static uint16_t steps_moved = 0;  // 记录已经移动的步数     第一次初始化的时候为0


    // 清除上一次的中止通知标志，防止误触发【避免本该关窗的时候残留的雨天信号影响当前状态】
    ulTaskNotifyValueClear(NULL, MOTOR_SIG_ABORT_TO_ZERO);     //不清楚pending状态 
    for(uint16_t i = 0; i < steps_to_move; i++)     //开始走步
    {
        
        uint32_t notify_value = 0;  //默认等于 0，表示没有通知
        if(xTaskNotifyWait(0, MOTOR_SIG_ABORT_TO_ZERO, &notify_value, 0) == pdTRUE)  //接收是否成功  参数四的0代表不进行阻塞等待
        {
            if(notify_value & MOTOR_SIG_ABORT_TO_ZERO)  // 检查是否收到中止通知
            {
                printf("[motor]:中途有雨 开始关窗 需往回走%d步\r\n", steps_moved);
                while(steps_moved > 0)
                {
                    Stepper_Step(&g_motor, STEPPER_DIR_CCW); // 关窗方向
                    vTaskDelay(pdMS_TO_TICKS(3));
                    steps_moved--;  //执行一次减一次
                }
                Stepper_PowerOff(&g_motor);  // 关窗到位后断电释放
                s_current_percent = 0;      // 重置开度百分比
                node2_state.louver_current_percent = s_current_percent; // 同步全局状态
                return;  // 中止流程，直接返回
            }
        }
        if(direction == STEPPER_DIR_CW)
        {
            steps_moved++;  
        }
        else if(direction == STEPPER_DIR_CCW)
        {
            if(steps_moved > 0)steps_moved--;
        }
        Stepper_Step(&g_motor, direction);
        vTaskDelay(pdMS_TO_TICKS(18)); 
    }
    // 转到位后断电释放
    Stepper_PowerOff(&g_motor);

    s_current_percent = target_percent;
    node2_state.louver_current_percent = s_current_percent;
}



//定义静态全局变量互斥锁 
static SemaphoreHandle_t uart_mutex = NULL; 

void sys_log_init()
{
    if(uart_mutex == NULL)
    {
        uart_mutex = xSemaphoreCreateMutex();   //创建互斥锁
    }
}


void safe_printf(const char *format, ...)
{
    if (xSemaphoreTake(uart_mutex, portMAX_DELAY) == pdTRUE)//无限等锁
    {
        va_list args;       //定义句柄对象用于寻址
        va_start(args, format); //锚定地址
        //在执行到vprintf时会根据内部的字符格式化情况从args取需要的参数
        vprintf(format, args); // 走重定向  vprintf第二个参数本身就是 va_list 类型
        va_end(args);
        xSemaphoreGive(uart_mutex); // 打完收工，放锁
    }
}


