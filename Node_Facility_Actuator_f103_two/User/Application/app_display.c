#include "app_display.h"
#include "cmsis_os2.h"
#include "stm32f103xb.h"

#include <stdint.h>
#include <stdio.h>

#include "key.h"
#include "stm32f1xx_hal.h"
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
    //按键初始化
    for (uint8_t k=0; k<key_count; k++) {
        key_attach_callback(&key_device[k], key_trigger_handle);
    }

    //关闭printf 行缓冲机制
    setvbuf(stdout, NULL, _IONBF, 0);


    //uart_dma初始化
    ringbuf_init(&buf_one,test,128);

}



void App_Show_Task(void *argument)
{
  /* USER CODE BEGIN App_Show_Task */
  /* Infinite loop */
  for(;;)
  {
    keyscan();
    osDelay(10);
  }
  /* USER CODE END App_Show_Task */
}


    static realData_t r_data;

void Data_Acquire_Task(void *argument)
{
  /* USER CODE BEGIN Data_Acquire_Task */
  /* Infinite loop */
  for(;;)
  {

    while (ringbuf_pop(&buf_one, &ok))
    {
        //这里不能用while 否则最后一帧数据的最后一次时返回true时 该字节会被传入两次 
        //一个字节被判了两包
        if(parse_byte(ok, &r_data))     //如果成功   
        {
            printf("r_data = %d + %d",r_data.data[0],r_data.data[1]);
            HAL_Delay(1000);
        }
    }
    osDelay(1);
  }
  /* USER CODE END Data_Acquire_Task */
}
