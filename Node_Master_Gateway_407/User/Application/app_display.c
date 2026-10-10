#include "app_display.h"
#include "FreeRTOS.h"

#include "cmsis_os2.h"
#include "ff.h"
#include "integer.h"
#include "lcdxx_driver.h"
#include "portmacro.h"
#include "projdefs.h"
#include "spi.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>
#include "string.h"
#include "function.h"
#include "FreeRTOSConfig.h"
#include <stdio.h>
#include "ecode.h"
#include "key.h"
#include "ds1302.h"


//任务中优先级最高
void vCont_task(void *argument)
{
  /* USER CODE BEGIN vCont_task */
  frtos_app_init();
    TickType_t now_tick = xTaskGetTickCount();  //获取当前tick
  /* Infinite loop */
  for(;;)
  {


    extern key_t key[4];
    key_scan(key,4);
//    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    vTaskDelayUntil(&now_tick, pdMS_TO_TICKS(20)); //绝对延时阻塞
  }
  /* USER CODE END vCont_task */

}

void vUI_task(void *argument)
{
  /* USER CODE BEGIN UI_display_task */
  /* 屏幕已在 app_init() 初始化，这里只负责绘制 */
  /* Infinite loop */
  TickType_t now_tick = xTaskGetTickCount();  //获取当前tick
  for(;;)
  {
      // HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8); // 心跳 LED
      Ds1302_Update_Time(&ds1302_one);  //更新ds1302对象数据
      s_printf("date:20%d-%d-%d\n\t%d-%d-%d | 周%d",ds1302_one.time_data.year,ds1302_one.time_data.mon,ds1302_one.time_data.day,
      ds1302_one.time_data.hour,ds1302_one.time_data.min,ds1302_one.time_data.sec,ds1302_one.time_data.week);
       vTaskDelayUntil(&now_tick, pdMS_TO_TICKS(900)); //绝对延时阻塞
  }
  /* USER CODE END UI_display_task */
}



//要注意栈溢出
  FATFS SD;       //
    FIL file_one;   //定义文件句柄 字节过大不可放入task中 容易撑爆栈内存 有几百字节 


char tfread_buffer[128] __attribute__((aligned(4)))= {0}; //tf卡读取 接收缓冲区（确保清零） 注意内存对齐
void vData_task(void *argument)
{
  /* USER CODE BEGIN data_acquire */
   
  extern FATFS SD;
  extern FIL file_one;
  FRESULT res;    //存储函数失败/成功结果
  vTaskDelay(500);

  res=f_mount(&SD, "0:", 1);  //立即挂载
  if(res!=FR_OK)s_printf("f_mount:err");
  UINT bw;

  res=f_open(&file_one, "0:/sha_dan.txt",FA_CREATE_ALWAYS | FA_WRITE );
  if(res!=FR_OK)s_printf("f_open:err");
  char data[]="周子弱大傻蛋！！！！";


  res=f_write(&file_one, data, strlen(data), &bw);
  if(res!=FR_OK)s_printf("f_write:err"),s_printf("write res=%d bw=%d\r\n", res, bw);
  res=f_close(&file_one);
  if(res!=FR_OK)s_printf("f_close:err");
  /* Infinite loop */
  for(;;)
  {
 //lcdxx_fill(&st7735_one, RED);
  if (res==FR_OK) {
    UINT br;  //用来记录实际读了多少字节
    s_printf("写入成功,写了%d字节\r\n",bw);
    f_open(&file_one, "0:/sha_dan.txt", FA_READ);
    f_read(&file_one, tfread_buffer, sizeof(tfread_buffer)-1 , &br);
    tfread_buffer[br] = '\0';
    s_printf("\r\n读取成功-内容为:\r\n \t%s | 字节数为%d",tfread_buffer,br);
    f_close(&file_one);
  }
    osDelay(1000);
  }
  /* USER CODE END data_acquire */
}

/*
10.6号 要将 各个写入的方式文件打开方式 以及文件指针 文件系统搞定；  √
不卡bug的情况下 将ec11消息队列完成          
*/
void vProcess_task(void *argument)
{
  /* USER CODE BEGIN vprocess_task */

  /* Infinite loop */
  for(;;)
  {
    extern osThreadId_t Cont_TaskHandle;
    UBaseType_t f_stack = uxTaskGetStackHighWaterMark(Cont_TaskHandle);
   s_printf("vcont_task %lu 字 ",f_stack);
    osDelay(1000);
  }
  /* USER CODE END vprocess_task */
}

