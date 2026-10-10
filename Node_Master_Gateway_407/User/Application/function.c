#include "function.h"
#include "ecode.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/cdefs.h>

#include "key.h"
#include "delay.h"


SemaphoreHandle_t uart_mux=NULL;    //建立句柄变量
void uartmux_init()
{
    if (uart_mux==NULL) {
        uart_mux = xSemaphoreCreateMutex(); //建锁
    }
}

void s_printf(const char * format,...)
{
    if(xSemaphoreTake(uart_mux, portMAX_DELAY)==pdTRUE) //一直等待取锁
    {
        va_list arg;    //定义句柄
        va_start(arg, format);
        vprintf(format, arg);
        va_end(arg);
      //  fflush(stdout);
        xSemaphoreGive(uart_mux);       //还锁
    }
    
}

//配合spi的DMA中断
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance==SPI2)
  {
     lcd_dma_tx_cplt_handler();
 
  }
}



//spi回调函数
uint8_t spi_callback(void *spix_t,uint8_t data)
{
  uint8_t rx;
    SPI_HandleTypeDef * spi_obj = (SPI_HandleTypeDef *)spix_t;  //声明类型
    HAL_SPI_TransmitReceive(spi_obj,&data,&rx,1,100);
    return rx;
}

/**
 * @brief EC11回调函数 做业务处理
 * 
 * @param dev   传入对象
 * @param event 具体事件
 */
void ecode_trigger(ecode_dev * dev,ecode_state event)
{
    /*业务处理*/
    switch (event) 
    {
        case cw:     //顺时针
  //       HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
        break;

        case ccw:     //逆时针
//         HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_12);
        break;

        case long_press:     //长按
 HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);
        break;

        case single_click:     //单击 
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_12);
        break;
    }
}



void key_trigger(key_t *dev,key_state event)
{
    switch (event)
    {
        case key_single_click:
             HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
        ;break;
        case key_double_click:
        
        ;break;
        case key_triple_click:
        
        ;break;
        case key_long_press:
             HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
        ;break;
    }
}

key_t key[4]={
    {GPIOD, GPIO_PIN_4,NULL,1,1},    //这里绑定了GPIOD4 为按键1
    {GPIOD, GPIO_PIN_5,NULL,1,1},
    {GPIOD, GPIO_PIN_6,NULL,1,1},
    {GPIOD, GPIO_PIN_7,NULL,1,1}
};


lcdxx_dev st7735_one;
ecode_dev ecode_one;    //定义编码器对象
ds1302_t ds1302_one;  //定义ds1302对象

void frtos_app_init()
{
    //开启DWT延时
    dwt_init();
    //关闭行缓冲
    setvbuf(stdout, NULL, _IONBF, 0); // _IONBF 表示无缓冲 (No Buffer)

    uartmux_init();     //初始化互斥锁

    //初始化屏幕
    lcdxx_dev_init(&st7735_one, &hspi2, spi_callback,
                   128, 160, 132, 162, 2, 1,
                   GPIOD, GPIO_PIN_10,   // RES
                   GPIOD, GPIO_PIN_9,    // DC
                   GPIOB, GPIO_PIN_12,   // CS
                   GPIOD, GPIO_PIN_8);   // BLK
    st7735_init(&st7735_one);
    lcdxx_fill(&st7735_one, BLACK);   //刷黑

    //初始化ecode-旋转编码器
    ecode_init(&ecode_one, GPIOE, GPIO_PIN_4, GPIOE, GPIO_PIN_5, GPIOE, GPIO_PIN_6);
    //为ecode对象绑定回调函数
    register_ecode_callback(&ecode_one, ecode_trigger);

    HAL_TIM_Base_Start_IT(&htim6);  //开启中断

    //给按键1绑定回调函数 
    key_attach_callback(&key[0],key_trigger);

    //初始化ds1302
    Ds1302_init(&ds1302_one,GPIOA,GPIO_PIN_4,GPIOA,GPIO_PIN_5,GPIOA,GPIO_PIN_6);

}

