#include "ds1302.h"
#include "gpio.h"
#include <stdint.h>
#include "delay.h"

//引脚变化较慢-无需操作寄存器（直接调用HAL库函数即可）
#define ds1302_CLK_H(x) HAL_GPIO_WritePin(x->CLK_PORT, x->CLK_PIN, GPIO_PIN_SET)
#define ds1302_CLK_L(x) HAL_GPIO_WritePin(x->CLK_PORT, x->CLK_PIN, GPIO_PIN_RESET)

#define ds1302_SDA_H(x) HAL_GPIO_WritePin(x->SDA_PORT, x->SDA_PIN, GPIO_PIN_SET)
#define ds1302_SDA_L(x) HAL_GPIO_WritePin(x->SDA_PORT, x->SDA_PIN, GPIO_PIN_RESET)

#define ds1302_RES_H(x) HAL_GPIO_WritePin(x->RES_PORT, x->RES_PIN, GPIO_PIN_SET)
#define ds1302_RES_L(x) HAL_GPIO_WritePin(x->RES_PORT, x->RES_PIN, GPIO_PIN_RESET)

#define  ds1302_get_sda(x) HAL_GPIO_ReadPin(x->SDA_PORT,x->SDA_PIN)     //读引脚

void Ds1302_write(ds1302_t *dev,uint8_t byte)
{
    for (uint8_t i=0; i<8; i++) 
    {
        ds1302_CLK_L(dev);
        if(byte & 0x01) //低字节优先
        {
            ds1302_SDA_H(dev);  //1
        }
        else {
            ds1302_SDA_L(dev);
        }
        delay_us(1);    //给芯片反应时间
        ds1302_CLK_H(dev);  //拉高-推入
        delay_us(1);    //给芯片反应时间
        byte >>= 1;  
    }
    ds1302_CLK_L(dev);  //写完拉低              //这里拉低 读取时第一个下降沿不要给 
    delay_us(1);    //给芯片反应时间
}

/**
 * @brief  如果接5v则不用延时 否则需延时1us
 * 
 * @param dev 
 * @return uint8_t 
 */
uint8_t Ds1302_read(ds1302_t *dev)
{
    uint8_t data=0x00;
    ds1302_SDA_H(dev);//释放总线        
    for (uint8_t i=0; i<8; i++) 
    {
        /*看情况这之间需要延时*/
       delay_us(1);
       ds1302_CLK_L(dev);

       if(ds1302_get_sda(dev))  //判断是高/低       读取时 低字节优先
       {       
            data |=(0x01<<i);
       }
       ds1302_CLK_H(dev); //拉高
       delay_us(1);
      
    }
    ds1302_CLK_L(dev);
    return data;
}

void Ds1302_Write_Data(ds1302_t *dev,addr_write instruct,uint8_t data)
{
    ds1302_RES_H(dev);  //拉高
    delay_us(1); 
    Ds1302_write(dev, instruct);    //写命令
    Ds1302_write(dev, data);        //写数据
    ds1302_RES_L(dev);  //结束
    delay_us(2); 
}

uint8_t Ds1302_Read_Data(ds1302_t *dev,addr_write instruct)
{
    uint8_t data;
    ds1302_RES_H(dev);  //拉高
    delay_us(1); 
    Ds1302_write(dev, instruct|0x01);    //写命令
    data = Ds1302_read(dev);  //读数据
    ds1302_RES_L(dev);
    delay_us(2); 
    return data;    //将数据弹出
    
}

void  Ds1302_Set_Time(ds1302_t * dev,date_t * time)
{
    Ds1302_Write_Data(dev, addr_control, write_on); // 关闭写保护
    Ds1302_Write_Data(dev, addr_sec, get_bcd(time->sec)&0x7f);  //写入秒数时要注意位7为1-禁止计时 (为1为低功耗时间暂停)
    Ds1302_Write_Data(dev, addr_min, get_bcd(time->min));
    Ds1302_Write_Data(dev, addr_hour, get_bcd(time->hour));
    Ds1302_Write_Data(dev, addr_day, get_bcd(time->day));
    Ds1302_Write_Data(dev, addr_mon, get_bcd(time->mon));
    Ds1302_Write_Data(dev, addr_week,   get_bcd(time->week));
    Ds1302_Write_Data(dev, addr_year, get_bcd(time->year));
    Ds1302_Write_Data(dev, addr_control,write_off); //  打开写保护
}

void Ds1302_Update_Time(ds1302_t * dev)
{
    dev->time_data.year = get_dec(Ds1302_Read_Data(dev, addr_year));
    dev->time_data.mon  = get_dec(Ds1302_Read_Data(dev, addr_mon)  ); 
    dev->time_data.day  = get_dec(Ds1302_Read_Data(dev, addr_day) ); 
    dev->time_data.hour = get_dec(Ds1302_Read_Data(dev, addr_hour) );
    dev->time_data.min  = get_dec(Ds1302_Read_Data(dev, addr_min)  ); 
    dev->time_data.sec  = get_dec(Ds1302_Read_Data(dev, addr_sec)  ); 
    dev->time_data.week = get_dec(Ds1302_Read_Data(dev, addr_week) ); 
}



void Ds1302_init(ds1302_t * dev,GPIO_TypeDef * clk_port,uint16_t clk_pin,GPIO_TypeDef * sda_port,
                uint16_t sda_pin,GPIO_TypeDef * res_port,uint16_t res_pin)
{
    dev->CLK_PORT=clk_port;  dev->SDA_PORT = sda_port; dev->RES_PORT =res_port;
    dev->CLK_PIN =clk_pin;   dev->SDA_PIN=sda_pin;     dev->RES_PIN = res_pin;

    ds1302_RES_L(dev);
    ds1302_CLK_L(dev);    
    ds1302_SDA_L(dev);

    uint8_t sec_ch = Ds1302_Read_Data(dev, addr_sec);  //读取秒数寄存器 [注意这里用来判断最高位 所以不转为十进制]
    if(sec_ch & 0x80)  //位7为1 说明时钟第一次上电
    {               
                   //年 月 日 时 分 秒 星期
        date_t time={26,10,10,21,01,30,6};  //写入默认时间
        Ds1302_Set_Time(dev,&time);
    }

    Ds1302_Write_Data(dev, addr_control, write_on); // 关闭写保护
    Ds1302_Write_Data(dev,addr_charge,charge_off);
    Ds1302_Write_Data(dev, addr_control, write_off); // 关闭写保护

}
