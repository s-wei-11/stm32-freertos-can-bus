#include "soft_spi.h"
#include "cmsis_gcc.h"
#include <stdint.h>




/*
    设计应为 CS交由上层设备控制 以及映射
    spi每个从设备都有一个响应引脚,要做到同一时间只有一个响应
*/

/**
 * @brief 封装定义spi设备
 * @note  如果miso不用则定义为NULL pin为0
 * @param dev 
 * @param scl_port 
 * @param scl_pin 
 * @param mosi_port 
 * @param mosi_pin 
 * @param miso_port 
 * @param miso_pin 

 */
void soft_spi_init(spi_t * dev,    GPIO_TypeDef * scl_port,uint16_t   scl_pin,
                   GPIO_TypeDef * mosi_port,uint16_t mosi_pin,GPIO_TypeDef * miso_port,
                   uint16_t miso_pin)          
{
    dev->scl_port=scl_port;
    dev->scl_pin=scl_pin;
    dev->mosi_port=mosi_port;
    dev->mosi_pin=mosi_pin;
    dev->miso_port=miso_port;
    dev->miso_pin=miso_pin;

}

#define scl_high(x)  ((x->scl_port)->BSRR  = (x->scl_pin))
#define scl_low(x)   ((x->scl_port)->BSRR  = (uint32_t)(x->scl_pin) << 16u)
#define mosi_high(x) ((x->mosi_port)->BSRR = (x->mosi_pin))
#define mosi_low(x)  ((x->mosi_port)->BSRR = (uint32_t)(x->mosi_pin) << 16u)
//#define cs_high(x)   ((x->cs_port)->BSRR   = (x->cs_pin))
//#define cs_low(x)    ((x->cs_port)->BSRR   = (uint32_t)(x->cs_pin) << 16u)

//miso为读取线
#define miso_read(x) (!!((x->miso_port)->IDR & (x->miso_pin)) )



/**
 * @brief 单字节spi写入读取——全双工模式
 * @note  mode0模式  空闲为低，上升沿（读取/写入）
 * @param dev 
 * @param data            为要发送的数据
 * @return uint8_t        接收的数据
 */
uint8_t soft_spi_swap(spi_t *dev,uint8_t data)
{
    uint8_t byte=0x00;
    scl_low(dev);
    for (uint8_t i=0; i<8; i++) {

        //写入逻辑
        if(data&0x80)
        {
            mosi_high(dev);
        }
        else {
            mosi_low(dev);
        }
        data <<=1;
        scl_high(dev);   //推入写的数据
        __NOP();
        //读取逻辑
        if(dev->miso_port!=NULL)
        {
            if(miso_read(dev))
            {
                byte|=(0x80>>i);//1000 0000
            }//0不用管
        }
        scl_low(dev);
        __NOP();
    }
    scl_low(dev);
    return byte;
}