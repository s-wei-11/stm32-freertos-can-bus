#pragma once

#include "stm32f1xx.h"


#define key_count 3

typedef struct key_t key_t;

typedef enum {
    KEY_EVENT_NONE = 0, // 无动作（推荐作为状态机初始/复位状态）
    single_click,          // 单击
    short_press,           // 短按（若业务上与单击区分时使用）
    double_click,          // 双击
    triple_click,          // 三击
    long_press,            // 长按
    long_press_hold,       // 长按保持（可选：持续按下触发）
    button_release         // 按键释放（可选：弹起事件）
} key_events;


typedef void (*key_callback) (key_t * dev,key_events event) ;   //定义之后就变为一个变量

struct key_t {

    GPIO_TypeDef * port;
    uint32_t       pin;

    key_callback   callback;    //回调函数-用于处理业务

    uint8_t        state;
    uint8_t        last_state;

    uint32_t       press_tim;
    uint32_t       release_tim;  
    uint8_t        pre_record;  //连击记录 
    uint8_t        key_lock;

};
