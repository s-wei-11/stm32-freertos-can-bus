#ifndef __at24cxx_h_
#define __at24cxx_h_
#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"


//eeprom 不进行软件或硬件协议解绑   但是考虑两根线可挂载多设备 还是再次使用结构体进行设备抽象


typedef struct{
    //
    I2C_HandleTypeDef * iic_t;          //绑定当前开的硬件iic对象【必须用指针 不然直接拷贝内存过大】
    uint8_t     at24cxx_addr;           //设备地址 
    uint16_t     at24cxx_page_size;      //单页宽度
    uint16_t    at24cxx_total_size;     //总字节数
    uint32_t    bit_scale;              //寻址范围
} eeprom_t;




bool eeprom_init(eeprom_t * dev, I2C_HandleTypeDef * iic_obj,uint8_t addr_dev,uint16_t page_size,uint16_t total_size);
bool eeprom_read(eeprom_t *dev,uint16_t read_position,uint8_t *p_buf,uint16_t len);
bool eeprom_write(eeprom_t *dev,uint16_t write_position,const uint8_t * p_buf,uint16_t len);

#endif

