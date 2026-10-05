#pragma once
#include "stm32f4xx.h"
#include <stdint.h>


typedef struct{
    GPIO_TypeDef * scl_port;    //时钟线
    uint16_t       scl_pin;
    GPIO_TypeDef * mosi_port;   //输出
    uint16_t       mosi_pin;
    GPIO_TypeDef * miso_port;   //接收
    uint16_t       miso_pin;
}spi_t;




uint8_t soft_spi_swap(spi_t *dev,uint8_t data);
void soft_spi_init(spi_t * dev,    GPIO_TypeDef * scl_port,uint16_t   scl_pin,
                   GPIO_TypeDef * mosi_port,uint16_t mosi_pin,GPIO_TypeDef * miso_port,
                   uint16_t miso_pin);