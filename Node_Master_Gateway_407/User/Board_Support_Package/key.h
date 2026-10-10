#ifndef __key_h
#define __key_h
#include "stm32f407xx.h"
#include <stdint.h>
#include "gpio.h"

/*
* 设计思路，对所有按键进行解耦 并添加短按 长按 连击功能
key_t key[4]={
    {GPIOD, GPIO_PIN_4,NULL,1,1},    //这里绑定了GPIOD4 为按键1
    {GPIOD, GPIO_PIN_5,NULL,1,1},
    {GPIOD, GPIO_PIN_6,NULL,1,1},
    {GPIOD, GPIO_PIN_7,NULL,1,1}
};//这里直接在这里定义并绑定即可    除此之外参数值都会默认为0

*/



typedef struct key_dev key_t; //提前声明struct key_t

typedef enum{
    key_single_click=0,
    key_double_click,
    key_triple_click,
    key_short_press,  //短按
    key_long_press,
    key_none
}key_state;

typedef void (*key_callback)(key_t * key,key_state event);

struct key_dev
{
    GPIO_TypeDef* port;
    uint16_t pin;
    key_callback callback;    //回调变量

    uint8_t key_state;  //当前状态
    uint8_t key_last_state; //上次状态

   
    uint8_t key_lock;   //长按锁
    uint32_t press_time;    //按键按下时间
    uint32_t release_time;  //按键释放时间
    uint8_t record;   //按键连击记录

    
};



extern key_t key[];


void key_scan(key_t * key,uint8_t key_num);
void key_attach_callback(key_t * dev,key_callback handler_func);


#endif

