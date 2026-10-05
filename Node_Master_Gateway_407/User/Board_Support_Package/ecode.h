#pragma once
#include <stdint.h>
#include "stm32f4xx_hal.h"  // 包含基础硬件库
//封装引脚





//用来返回编码器状态
typedef enum{
    cw=0,       //顺时针
    ccw=1,      //逆时针
    long_press=2,       //长按
    short_press=3,      //短按
    single_click=4,  //单击
    double_click=5,    //双击
    triple_click=6,     //三击
    none       //无操作状态
}ecode_state;


typedef struct ecode_dev ecode_dev; //将struct ecode_dev 定义成ecode_dev

//声明回调函数
//这样声明后 ecode_callback 也就成为一种变量类型了
typedef void (*ecode_callback)(ecode_dev *dev,ecode_state event);

//如果这样定义用ecode_dev 前都需要 struct + ecode_dev
struct ecode_dev{
    GPIO_TypeDef* port_a;
    uint16_t pin_a;
    GPIO_TypeDef* port_b;
    uint16_t pin_b;
    GPIO_TypeDef* port_d;
    uint16_t pin_d;

    uint8_t pin_a_last;
    uint8_t pin_b_last;
    uint8_t pin_d_last; //为了后面复用所以才在这里定义此状态变量


    long int count;    //用来判断拧了几次 顺时针加 逆时针减

    uint32_t release_time; //松手时间 
    uint32_t press_time;    //按下时间
    uint8_t lock_long; //长按防触发锁
    uint8_t record; //记录连击几次

    int32_t speed;    //用来判断拧的速度
    uint32_t turn_time; //用来判断拧的时间 单位为hz


    //
    ecode_callback callback;//回调函数变量
};




void ecode_init(ecode_dev * dev,GPIO_TypeDef * port_a,uint16_t pin_a,GPIO_TypeDef * port_b,\
                uint16_t pin_b,GPIO_TypeDef * port_d,uint16_t pin_d);
ecode_state ecode_getstate(ecode_dev * dev);
void register_ecode_callback(ecode_dev * dev,ecode_callback callback);