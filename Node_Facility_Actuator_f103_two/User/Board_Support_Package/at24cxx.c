#include "at24cxx.h"
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_i2c.h"


#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>


/**
 * @brief   对设备对象进行初始化
 * 
 * @param dev         设备对象
 * @param iic_obj     链接协议对象
 * @param addr_dev    器件地址
 * @param page_size   页宽
 * @param total_size  总字节数
 */
bool eeprom_init(eeprom_t * dev, I2C_HandleTypeDef *iic_obj,uint8_t addr_dev,uint16_t page_size,uint16_t total_size)
{
    if (dev == NULL || iic_obj == NULL ||page_size == 0 || total_size == 0)return false;
    dev->iic_t  = iic_obj;
    dev->at24cxx_page_size    = page_size;
    dev->at24cxx_total_size   = total_size;
    dev->at24cxx_addr  = addr_dev;

    //超过8位 如c04 用了芯片的一个引脚去自动调节 而多余引脚只有3个 所以16以上只能用16bit来寻址
    /* AT24C01~C16(<=2048字节)为8位内部寻址；C32及以上(>2048字节)才为16位寻址 */
    if(total_size > 2048)
    {
        dev->bit_scale = I2C_MEMADD_SIZE_16BIT;
    }
    else 
    {
        dev->bit_scale = I2C_MEMADD_SIZE_8BIT;
    }

    return true;
}

/**
 * @brief 检测设备是否准备好
 * @note  在10ms内询问60次[会阻塞程序]
 * @return true             可读/写
 * @return false            不可
 */
static bool eeprom_is_ready(eeprom_t * dev)
{
    return (HAL_I2C_IsDeviceReady(dev->iic_t, dev->at24cxx_addr, 60, 10) == HAL_OK);
}

/**
 * @brief 读取数据
 * @note  硬件iic会自动或1
 * @param addr_t      要读取的位置
 * @param p_buf       接收字节用的变量
 * @param len         数据长度（字节数）
 * @return true       接收成功
 * @return false      读取失败
 */
bool eeprom_read(eeprom_t *dev,uint16_t read_position,uint8_t *p_buf,uint16_t len)
{
    if(dev == NULL || p_buf==NULL || len==0 || (read_position + len ) > dev->at24cxx_total_size)
    {
        return false;
    }
    if(!(eeprom_is_ready(dev)))
    {
        return false;
    }
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(dev->iic_t,dev->at24cxx_addr,read_position,dev->bit_scale,p_buf,len,100);
    if(status != HAL_OK)return false;
    return true;
}




/**
 * @brief 写入函数
 * 
 * @param dev 
 * @param write_position    初始写入的位置
 * @param p_buf             要写入的数据组
 * @param len               字节数
 * @return true 
 * @return false 
 */
bool eeprom_write(eeprom_t *dev,uint16_t write_position,const uint8_t * p_buf,uint16_t len)
{
    //需要考虑页写的情况    【 c02实际为 0-255 】   避免回卷
    //先判断要写多少？
    if (dev == NULL || p_buf == NULL || len == 0) return false;
    if((write_position + len) > dev->at24cxx_total_size )return false;  //写不下

    uint16_t remain_bytes = len;              // 剩余待写字节数
    uint16_t  cur_addr    = write_position;   // 当前写入的 EEPROM 物理地址
    const uint8_t *p_cur_data   = p_buf;            // 数据指针当前偏移   因为数据可能不是一次发送
    
    while(remain_bytes > 0)        
    {
        uint16_t page_offset = cur_addr % dev->at24cxx_page_size;    //当前页的偏移量        position 5  len 7
        uint16_t space_in_page = dev->at24cxx_page_size - page_offset;   //当前页可写空间    8-5=3

        //判断剩余字节数与当前页可写空间关系
        uint16_t chunk_len = (remain_bytes > space_in_page)? space_in_page : remain_bytes;  //如果大于剩余空间则先传 剩余空间
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(dev->iic_t,dev->at24cxx_addr, cur_addr ,dev->bit_scale,(uint8_t *)p_cur_data,chunk_len,100);
        
        if(status != HAL_OK )return false;
        if (!(eeprom_is_ready(dev))) return false;
        
        remain_bytes -= chunk_len;  //减去刚才写的字节数    
        cur_addr += chunk_len   ;//计算下一次要写的位置
        p_cur_data +=chunk_len ;    //推动指针偏移
    }
    return true;    //写入完毕
}

/*
处理回卷一次性发送 —— 核心骨架
1. 算这次能拿多少：chunk = min(还剩多少, 当前容器还能装多少)
2. 把 chunk 发出去
3. 移动账本：总量扣掉 chunk，地址向前加 chunk，指针向前推 chunk
*/

