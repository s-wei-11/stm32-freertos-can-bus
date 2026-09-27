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


static uint8_t test[128];   //数据池
ubuf_t buf_one;
uint8_t ok;


void app_init(void)
{
    //微秒延时初始化
    DWT_init();

    //按键初始化
    for (uint8_t k=0; k<key_count; k++) {
        key_attach_callback(&key_device[k], key_trigger_handle);
    }

    //关闭printf 行缓冲机制
    setvbuf(stdout, NULL, _IONBF, 0);


    //uart_dma初始化
    ringbuf_init(&buf_one,test,128);


    //雨滴传感器初始化

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


static realData_t r_data;
extern uint8_t notify_idle_flag;    
void Data_Acquire_Task(void *argument)
{
  /* USER CODE BEGIN Data_Acquire_Task */
  eeprom_t eeprom_one;
  eeprom_init(&eeprom_one, &hi2c2, 0xa0, 8, 256);
  char k[]="周紫若大傻蛋!";
  uint8_t data[20];
  if(eeprom_write(&eeprom_one, 3, (uint8_t *)k, strlen(k)))
  {
    //如果成功
    eeprom_read(&eeprom_one, 3, data, strlen(k));
  }
  /* Infinite loop */
  for(;;)
  {
  //  无限期休眠等待 IDLE 中断唤醒（CPU 占用率为 0%）
 // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if(notify_idle_flag)
    {
        notify_idle_flag = 0;   //通知到了就清0 

        //如果内部水池没抽完 由于while的特性 会让任务不进入阻塞态
        while (ringbuf_pop(&buf_one, &ok))
        {
            //这里不能用while 否则最后一帧数据的最后一次时返回true时 该字节会被传入两次 
            //一个字节被判了两包
            if(parse_byte(ok, &r_data))     //如果成功   
            {
                // printf("r_data = %d + %d",r_data.data[0],r_data.data[1]);
                for (uint8_t k=0; k<r_data.len; k++) {
                    printf("%d",r_data.data[k]);
                }
                vTaskDelay(1000);
            }
        }
    }
    printf("%s",data);
    osDelay(300);
  }
  /* USER CODE END Data_Acquire_Task */
}



Ds18bxx_t ds18b20_one;
void Deal_data(void *argument)
{
  /* USER CODE BEGIN Deal_data */
    float current_temperature = 0.0f;
    uint32_t fail_count = 0;
    if(DS18B20_Init(&ds18b20_one, GPIOB, GPIO_PIN_9))
    {
        printf("初始化成功!!");
    }
    else {
        printf("failure ");
    }

        // 实际接线：PA0=IN1(A), PA1=IN2(B), PA2=IN3(C), PA3=IN4(D)
    // 使用 main.h 中的宏，避免魔法数字，改引脚时只需改 CubeMX

  /* Infinite loop */
  for(;;)
  {

   // 向传感器下发温度转换指令 (耗时约 1ms)
        if (DS18B20_StartConversion(&ds18b20_one))
        {
            vTaskDelay(pdMS_TO_TICKS(750)); 

            // 步骤 C：读取转换后的数据 (耗时约 1ms)
            if (DS18B20_ReadTemp(&ds18b20_one, &current_temperature))
            {
                fail_count = 0; // 通信成功，清零故障计数

                // 打印或转交业务层（通过队列发送到 CAN 发送任务）
                printf("[Temp] Current: %.2f C\r\n", current_temperature);
                
                // 将 current_temperature 打包发送至 CAN 报文队列
            }
            else
            {
                fail_count++;
                printf("[Sensor Warning] Read scratchpad failed (count: %lu)\r\n", fail_count);
            }
        }
        else
        {
            fail_count++;
            printf("[Sensor Error] Start conversion failed - Device disconnected!\r\n");
        }

        // 步骤 D：如果连续 3 次通信失败，执行硬件故障降级/报警处理
        if (fail_count >= 3)
        {
            // 标记传感器断线，执行安全停机或向上位机上报故障码
            // CAN_Report_Error(ERR_TEMP_SENSOR_OFFLINE);
        }

        // 步骤 E：控制采样刷新周期（例如：每隔 1 秒总体采样一次）
        // 前面已经等待了 750ms，此处仅需再延时 250ms



                // 28BYJ-48 (8拍): 单步 5ms，输出轴转满一圈(4096拍)约 20s。
                // 本任务优先级为 Normal；若仍有丢步，可将高频脉冲控制移到
                // 定时器中断或更高优先级任务中生成。



        
         vTaskDelay(pdMS_TO_TICKS(1000));


    
  }
  /* USER CODE END Deal_data */
} 





//定义步进电机对象
Stepper_28BYJ48_t g_motor;
void device_control(void *argument)
{
  /* USER CODE BEGIN device_control */


    GPIO_TypeDef *ports[4] = {GPIOA,GPIOA,GPIOA,GPIOA}; //定义四个端口
    uint16_t pins[4] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3};   //定义四个引脚
    Stepper_Init(&g_motor, ports, pins);    //进行初始化绑定

    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
    TIM3->CCR2 = 30;    //
  /* Infinite loop */

  for(;;)
  {
       

    stepper_control();
    Stepper_PowerOff(&g_motor);

    osDelay(1000);
  }
  /* USER CODE END device_control */
}



