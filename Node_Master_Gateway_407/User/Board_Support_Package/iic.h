#ifndef __iic_h_
#define __iic_h_
#include "main.h"
#include <stdint.h>


/*
void iic_start(void);
void iic_stop(void);
void iic_send_byte(uint8_t data);
uint8_t iic_receive_byte(void);
void iic_send_ack(uint8_t x);
uint8_t iic_receive_ack(void);
*/


//核心任务是解决引脚复用 以及增加通信速率可控模式
typedef  struct{
    GPIO_TypeDef* scl_port;
    uint16_t      scl_pin;
    GPIO_TypeDef* sda_port;
    uint16_t      sda_pin;
    uint32_t      speed;    //250khz 只能填khz为单位 上限999khz
}softi2c_t;


void iic_init(softi2c_t * x,GPIO_TypeDef* scl_port,uint16_t scl_pin,GPIO_TypeDef* sda_port,uint16_t sda_pin,uint32_t speed);
void iic_start(softi2c_t * dev);
void iic_stop(softi2c_t* dev);
void iic_send_ack(softi2c_t * dev,uint8_t x);
int8_t iic_receive_ack(softi2c_t * dev);
void iic_send_byte(softi2c_t * dev,uint8_t data);
uint8_t iic_receive_byte(softi2c_t* dev);

#endif /* __iic_h_ */

