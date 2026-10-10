#pragma once
#include "stm32f407xx.h"
#include <stdint.h>
/*
    ⭐写此模块时需注意 写地址后读取 整体的时序
    且读写要加延时 不能看是ns就不管-ns就延时1us
*/
#define charge_on 0xaa  //开启充电
#define charge_off 0xa0 //关闭充电

#define write_on  0x00      //关闭写保护
#define write_off (write_on|0x80)   //开启写保护—禁止写入

/*      寄存器内部数据皆为BCD码         */
#define get_bcd(x)  ((((x)/10)<<4)+((x)%10))    //传入十进制数
#define get_dec(x)  (((x)>>4)*10+((x)&0x0f))    //传入BCD码

//寄存器地址
//读操作需要在寄存器地址上与1
// 1000 0000
// 8    842  
typedef enum{
    addr_sec = 0x80,
    addr_min = 0x82,
    addr_hour = 0x84,           //写入“小时” 的时候要注意 am pm 12/24小时制     “位7为0”-24小时制
    addr_day  = 0x86,
    addr_mon  = 0x88,
    addr_week = 0x8a,
    addr_year = 0x8c, 
    addr_charge = 0x90,  //充电寄存器地址
    addr_control = 0x8e  //控制寄存器
}addr_write;


typedef struct
{
    uint8_t year;       //年只能存00-99（也就是自己设置年代）
    uint8_t mon;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t week;
}date_t;

typedef struct {
    GPIO_TypeDef * CLK_PORT;
    uint16_t       CLK_PIN;
    GPIO_TypeDef * SDA_PORT;
    uint16_t       SDA_PIN;
    GPIO_TypeDef * RES_PORT;        
    uint16_t       RES_PIN;
    
    date_t         time_data;  //时间数据
}ds1302_t;

//需要时间直接从 ds1302对象里面去读取即可
void Ds1302_Update_Time(ds1302_t * dev);        //放入执行逻辑中 定时更新设备对象数据
void  Ds1302_Set_Time(ds1302_t * dev,date_t * time);    //设定时间
void Ds1302_init(ds1302_t * dev,GPIO_TypeDef * clk_port,uint16_t clk_pin,GPIO_TypeDef * sda_port,
                uint16_t sda_pin,GPIO_TypeDef * res_port,uint16_t res_pin);