#include "app_display.h"
#include "Delay.h"
#include "cmsis_os2.h"
#include "ds18b20.h"
#include "stm32f103xb.h"
// 补齐下面两行 FreeRTOS 原生支持：
#include "FreeRTOS.h"
#include "stm32f1xx_hal_gpio.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>

#include "key.h"

#include "uart_app.h"
#include "usart.h"



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
    
    osDelay(1);
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

  /* Infinite loop */
  for(;;)
  {

   // 向传感器下发温度转换指令 (耗时约 1ms)
        if (DS18B20_StartConversion(&ds18b20_one))
        {
            // 步骤 B：非阻塞等待硬件内部转换完成
            // 12-bit 分辨率最大需要 750ms，调用 vTaskDelay 彻底释放 CPU 给其他任务
            vTaskDelay(pdMS_TO_TICKS(750));

            // 步骤 C：读取转换后的数据 (耗时约 1ms)
            if (DS18B20_ReadTemp(&ds18b20_one, &current_temperature))
            {
                fail_count = 0; // 通信成功，清零故障计数

                // 打印或转交业务层（通过队列发送到 CAN 发送任务）
                printf("[Temp] Current: %.2f C\r\n", current_temperature);
                
                // TODO: 将 current_temperature 打包发送至 CAN 报文队列
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
        vTaskDelay(pdMS_TO_TICKS(250));
    
  }
  /* USER CODE END Deal_data */
}