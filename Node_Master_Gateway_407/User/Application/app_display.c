#include "app_display.h"
#include "FreeRTOS.h"

#include "cmsis_os2.h"
#include "ff.h"
#include "integer.h"
#include "lcdxx_driver.h"
#include "portmacro.h"
#include "spi.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>
#include "string.h"
#include "function.h"
#include "FreeRTOSConfig.h"
#include <stdio.h>

//配合spi的DMA中断
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance==SPI2)
  {
     lcd_dma_tx_cplt_handler();
 
  }
}


lcdxx_dev st7735_one;
lcdxx_dev st7789_one;
//spi回调函数
uint8_t spi_callback(void *spix_t,uint8_t data)
{
  uint8_t rx;
    SPI_HandleTypeDef * spi_obj = (SPI_HandleTypeDef *)spix_t;  //声明类型
    HAL_SPI_TransmitReceive(spi_obj,&data,&rx,1,100);
    return rx;
}

void app_init()
{

  setvbuf(stdout, NULL, _IONBF, 0); // _IONBF 表示无缓冲 (No Buffer)

    lcdxx_dev_init(&st7735_one, &hspi2, spi_callback,
                   128, 160, 132, 162, 2, 1,
                   GPIOD, GPIO_PIN_10,   // RES
                   GPIOD, GPIO_PIN_9,    // DC
                   GPIOB, GPIO_PIN_12,   // CS
                   GPIOD, GPIO_PIN_8);   // BLK

st7735_init(&st7735_one);

    lcdxx_fill(&st7735_one, RED);   // 满屏红
    HAL_Delay(200);
     lcdxx_fill(&st7735_one, YELLOW);   // 满屏红
       HAL_Delay(200);
      lcdxx_fill(&st7735_one, WHITE);   // 满屏红


}

void vCont_task(void *argument)
{
  /* USER CODE BEGIN vCont_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10000);
  }
  /* USER CODE END vCont_task */

}

void UI_display_task(void *argument)
{
  /* USER CODE BEGIN UI_display_task */
  /* 屏幕已在 app_init() 初始化，这里只负责绘制 */
  /* Infinite loop */
  for(;;)
  {
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8); // 心跳 LED

        lcdxx_dma_async_fill(&st7735_one, RED);
        lcdxx_show_chinese(&st7735_one, 0, line1, "孙大王", RED, WHITE);
        
        osDelay(100);

        lcdxx_dma_async_fill(&st7735_one, YELLOW);
        osDelay(1000);

        lcdxx_dma_async_fill(&st7735_one, BLUE);
        osDelay(1000);

        lcdxx_dma_async_fill(&st7735_one, 0xf8f8);
        osDelay(1000);
  }
  /* USER CODE END UI_display_task */
}



//要注意栈溢出
  FATFS SD;       //
    FIL file_one;   //定义文件句柄 字节过大不可放入task中 容易撑爆栈内存 有几百字节 


char tfread_buffer[128] = {0}; //tf卡读取 接收缓冲区（确保清零）
void data_acquire(void *argument)
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


void vprocess_task(void *argument)
{
  /* USER CODE BEGIN vprocess_task */
   uartmux_init();
  /* Infinite loop */
  for(;;)
  {
    // extern osThreadId_t UI_TaskHandle, Receive_TaskHandle;
    // UBaseType_t f_stack = uxTaskGetStackHighWaterMark( Receive_TaskHandle);
  //  s_printf("ui_task %lu 字 ",f_stack);
    osDelay(1000);
  }
  /* USER CODE END vprocess_task */
}

