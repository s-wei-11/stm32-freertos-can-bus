#include "iic.h"
#include "delay.h"

#include "stm32f4xx_hal_gpio.h"
#include <stdint.h>

#define IIC_SCL_H(x) HAL_GPIO_WritePin(x->scl_port, x->scl_pin, GPIO_PIN_SET)
#define IIC_SCL_L(x) HAL_GPIO_WritePin(x->scl_port, x->scl_pin, GPIO_PIN_RESET)
#define IIC_SDA_H(x) HAL_GPIO_WritePin(x->sda_port, x->sda_pin, GPIO_PIN_SET)
#define IIC_SDA_L(x) HAL_GPIO_WritePin(x->sda_port, x->sda_pin, GPIO_PIN_RESET)
//#define delay_us(x)  delay_us(x->speed)

void iic_init(softi2c_t * x,GPIO_TypeDef* scl_port,uint16_t scl_pin,
                GPIO_TypeDef* sda_port,uint16_t sda_pin,uint32_t speed)
{// 此驱动函数定义在 iic.c 中，其调用语句必须放在 main() 函数的主循环 while(1) 之前
    uint32_t hz=500/speed;
    if(hz<1)hz=1;
    x->scl_port=scl_port;
    x->scl_pin=scl_pin;
    x->sda_port=sda_port;
    x->sda_pin=sda_pin;
    x->speed=hz;
}

void iic_start(softi2c_t * dev)
{
    IIC_SDA_H(dev);
    IIC_SCL_H(dev);
    delay_us(dev->speed);
    IIC_SDA_L(dev);
    delay_us(dev->speed);
    IIC_SCL_L(dev);

}
void iic_stop(softi2c_t* dev)
{
    IIC_SCL_L(dev);
    IIC_SDA_L(dev);
    delay_us(dev->speed);
    IIC_SCL_H(dev);
    delay_us(dev->speed);
    IIC_SDA_H(dev);
    delay_us(dev->speed);
}

/**
 * @brief 主机发送应答信号
 * 
 * @param dev 
 * @param x 发送非0为不响应  发送0为响应
 */

void iic_send_ack(softi2c_t * dev,uint8_t x)
{
    IIC_SCL_L(dev);
    //delay_us(dev->speed); //删除
    if(x)
    {
        IIC_SDA_H(dev);
    }
    else {
    IIC_SDA_L(dev);
    }
    delay_us(dev->speed); //改善 要给sda反应时间
    IIC_SCL_H(dev);
    delay_us(dev->speed);
    IIC_SCL_L(dev);
    delay_us(dev->speed);
}

/**
 * @brief   iic设备状态响应函数
 * 
 * @param dev 
 * @return int8_t 从机响应返回0 不响应返回-1
 */
int8_t iic_receive_ack(softi2c_t * dev)
{
    uint8_t k;
    IIC_SDA_H(dev);
    IIC_SCL_L(dev);
    delay_us(dev->speed);
    IIC_SCL_H(dev);//接收时 scl要我们先拉起 后再读取sda
    delay_us(dev->speed);
    k=HAL_GPIO_ReadPin(dev->sda_port, dev->sda_pin);
    IIC_SCL_L(dev);//除了开始或结束 其余要保持scl为低 避免数据有错
    delay_us(dev->speed);
    if(k)
    {
        return -1;
    }
    return k;
}

/**
 * @brief iic发送数据 根据手册可知 数据高位先行
 * 
 * @param dev 
 * @param data 
 */
void iic_send_byte(softi2c_t * dev,uint8_t data)
{
    for(uint8_t x=0;x<8;x++)
    {
        IIC_SCL_L(dev);
        //delay_us(dev->speed); //删除
        if(data&0x80)
        {
            IIC_SDA_H(dev);
        }
        else {
            IIC_SDA_L(dev);
        }
        delay_us(dev->speed); //要给sda反应时间
        IIC_SCL_H(dev);
        delay_us(dev->speed);
        data=data<<1;
    }
    IIC_SCL_L(dev);
}

uint8_t iic_receive_byte(softi2c_t* dev)
{
    uint8_t byte=0x00;
    IIC_SDA_H(dev);
    for (uint8_t i=0; i<8; i++) 
    {
        IIC_SCL_H(dev);
        delay_us(dev->speed);
        byte|=(HAL_GPIO_ReadPin(dev->sda_port, dev->sda_pin)<<(7-i));
        IIC_SCL_L(dev);
        delay_us(dev->speed);
    }
    return byte;
}

/*
void iic_start(void)
{
    IIC_SDA_H;
    IIC_SCL_H;
    delay_us(2);
    IIC_SDA_L;
    delay_us(2);
    IIC_SCL_L;

void iic_stop(void)
{
    IIC_SCL_L;
    IIC_SDA_L;
    delay_us(2);
    IIC_SCL_H;
    delay_us(2);
    IIC_SDA_H;
    delay_us(2);
}

void iic_send_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        IIC_SCL_L;
        if (data & 0x80)
        {
            IIC_SDA_H;  //数据线 置一
        }
        else
        {
            IIC_SDA_L; //数据线 置零
        }
        data <<= 1;
        delay_us(2);
        IIC_SCL_H;
        delay_us(2);
    }
    IIC_SCL_L;
}

uint8_t iic_receive_byte(void)
{
    uint8_t data = 0x00;    // 0000 0000
    IIC_SDA_H; // Release SDA for input  就是释放sda控制权 给从设备写数据
    for (int i = 0; i < 8; i++)
    {
        IIC_SCL_L;
        delay_us(2);
        IIC_SCL_H;
        data |= (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) << (7 - i)); // Read SDA and shift into data
        delay_us(2);
    }
    IIC_SCL_L;
    return data;
}   



void iic_send_ack(uint8_t x)    // x=0,发送ACK; x=1,发送NACK
{
    IIC_SCL_L;
    if (x)
    {
        IIC_SDA_H; // Send NACK
    }
    else
    {
        IIC_SDA_L; // Send ACK
    }
    delay_us(2);
    IIC_SCL_H;
    delay_us(2);
    IIC_SCL_L;
}

uint8_t iic_receive_ack(void)
{
    uint8_t ack;
    IIC_SCL_L;
    IIC_SDA_H; // Release SDA for input 释放总线
    delay_us(2);
    IIC_SCL_H;
    delay_us(2);
    ack = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1); // Read ACK/NACK from slave
    IIC_SCL_L;
    return ack;
}

}
*/

