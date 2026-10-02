#include "app_display.h"
#include "Delay.h"
#include "cmsis_os2.h"
#include "ds18b20.h"
#include "i2c.h"
#include "stm32f103xb.h"
// 补齐下面两行 FreeRTOS 原生支持：
#include "FreeRTOS.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>

#include "key.h"
#include "step_28byj48.h"
 #include "tim.h"
#include "uart_app.h"
#include "usart.h"
#include "function.h"
#include "at24cxx.h"
#include <string.h>

uint8_t i[2]={0,0};

void key_trigger_handle(key_t * dev,key_events event)
{
    switch (dev->key_id)
    {
        case 0:
            if(event==single_click)
            {
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
                i[0]++;i[1]++;
            }
            else if (event==double_click) {
                 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);

                 //发送包
                 packet_send(&huart1,i,sizeof(i));
            }
            else if (event == triple_click) {
            
            }
            else if (event == long_press) {
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
            }
        ;break;
        case 1:;break;
        case 2:;break;
    }
}


static uint8_t test[128];   //uart数据池
ubuf_t buf_one;             //uart数据缓冲区对象
uint8_t ok;                 

eeprom_t eeprom_one;    
void app_init(void)
{
    //微秒延时初始化
    DWT_init();

    //按键初始化
    for (uint8_t k=0; k<key_count; k++) {
        key_attach_callback(&key_device[k], key_trigger_handle);
    }

    //关闭printf 行缓冲机制54
    setvbuf(stdout, NULL, _IONBF, 0);


    //uart_dma初始化
    ringbuf_init(&buf_one,test,128);


      eeprom_init(&eeprom_one, &hi2c2, 0xa0, 8, 256);



   GPIO_TypeDef *ports[4] = {GPIOA,GPIOA,GPIOA,GPIOA}; //定义四个端口
    uint16_t pins[4] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3};   //定义四个引脚
    extern Stepper_28BYJ48_t g_motor;
    Stepper_Init(&g_motor, ports, pins);    //进行初始化绑定

    // 节点 2 设备初始化
    node2_device_init();

    //步进电机初始化
   // Stepper_Zero_Calibrate();


}


//高优先级任务
void App_Show_Task(void *argument)
{
  /* USER CODE BEGIN App_Show_Task */


  /* Infinite loop */
  for(;;)
  {
    keyscan();
    osDelay(20);
  }
  /* USER CODE END App_Show_Task */
}



extern uint8_t notify_idle_flag; 

// 引入外部任务句柄 
extern osThreadId_t DeviceHandle;
void Data_Acquire_Task(void *argument)
{
  /* USER CODE BEGIN Data_Acquire_Task */


    static bool last_rain_state = false;  //上一次默认为无雨
    static uint8_t last_motor_percent = 255;  //上一次目标开度 ；初始随便定义一个

  /* Infinite loop */
  for(;;)
  {
    node2_state_update(&node2_state);//温度更新
    node2_apply_outputs();    // 根据当前天气和温度等级，计算并应用执行器输出
    
    if(node2_state.rain_state != last_rain_state  && last_rain_state == false) //检测下雨状态变化
    {
        //通过任务通知打断电机
        xTaskNotify((TaskHandle_t)DeviceHandle, MOTOR_SIG_ABORT_TO_ZERO, eSetBits);
    }
    last_rain_state = node2_state.rain_state;
  if(node2_state.louver_target_percent != last_motor_percent)
  {
    last_motor_percent = node2_state.louver_target_percent;  //更新目标开度
    xTaskNotify((TaskHandle_t)DeviceHandle,MOTOR_SIG_START_MOVE , eSetBits);  //打入通知
  }
    osDelay(200);
  }
  /* USER CODE END Data_Acquire_Task */
}


//温度获取任务
Ds18bxx_t ds18b20_one;
void Deal_data(void *argument)
{
  /* USER CODE BEGIN Deal_data */

  sys_log_init();
    if(DS18B20_Init(&ds18b20_one, GPIOB, GPIO_PIN_9))
    {
        printf("初始化成功!!");
    }
    else {
        printf("failure ");
        
    }
  /* Infinite loop */
  for(;;)
  {

   // 向传感器下发温度转换指令 (耗时约 1ms)
    get_temp(&node2_state ,&ds18b20_one);

        // 步骤 E：控制采样刷新周期（例如：每隔 1 秒总体采样一次）
        // 前面已经等待了 750ms，此处仅需再延时 250ms


         vTaskDelay(pdMS_TO_TICKS(250));


    
  }
  /* USER CODE END Deal_data */
} 




//定义步进电机对象
Stepper_28BYJ48_t g_motor;



void device_control(void *argument)
{
  /* USER CODE BEGIN device_control */



   
  /* Infinite loop */
uint32_t notify_value = 0;  //默认等于 0，表示没有通知
  for(;;)
  {
       if(xTaskNotifyWait(0, MOTOR_SIG_START_MOVE, &notify_value, portMAX_DELAY)==pdTRUE) //并消除原本的通知位  再任务没有通知的时候 挂起
       {
          if(notify_value & MOTOR_SIG_START_MOVE)
          {
              // 处理启动电机通知
                Stepper_Louver_Control(node2_state.louver_target_percent);
          }
       }//有了这个事件来驱动 就不用再使用 osDelay 
  }
  /* USER CODE END device_control */
}



