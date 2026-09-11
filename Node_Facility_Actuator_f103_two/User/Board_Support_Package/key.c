#include "key.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>


key_t key_device[key_count]={{GPIOB,GPIO_PIN_13,NULL,1,1},{GPIOB,GPIO_PIN_14,NULL,1,1},{GPIOB,GPIO_PIN_15,NULL,1,1}};   //给按键初始化

void key_attach_callback(key_t * dev,key_callback function)
{
    dev->callback=function; //绑定业务处理函数
}

static inline void notify_event(key_t * dev,key_events event)
{
    if (dev->callback != NULL) {
        dev->callback(dev,event);
    }
}


void keyscan()
{
    for(uint8_t i=0; i<key_count ; i++)
    {
        uint32_t now_time=HAL_GetTick();

        key_device[i].state=HAL_GPIO_ReadPin(key_device[i].port, key_device[i].pin);
        if(key_device[i].state == 0 && key_device[i].last_state == 1)   //下降沿
        {
            key_device[i].press_tim = now_time;
            key_device[i].key_lock  = 1;    //解锁
            key_device[i].pre_record++; //记录按下次数
        }
        if(key_device[i].state == 0 && key_device[i].last_state == 0)
        {
            if(now_time-key_device[i].press_tim >= 1500 && key_device[i].key_lock == 1)
            {
                key_device[i].key_lock=0;   //上锁
                key_device[i].pre_record=0;
                notify_event(&key_device[i], long_press);
            }
        }
        if(key_device[i].state == 1 && key_device[i].last_state == 0)
        {
            key_device[i].release_tim = now_time;
        } 
        if(key_device[i].state == 1 && key_device[i].last_state == 1) 
        {
            if (now_time-key_device[i].release_tim >= 200 && key_device[i].pre_record>=1) {
                switch (key_device[i].pre_record) {
                    case 1 : 
                        notify_event(&key_device[i], single_click);
                    ;break;;
                    case 2 : 
                        notify_event(&key_device[i], double_click);
                    ;break;;
                    case 3 :
                        notify_event(&key_device[i], triple_click);
                    ;break;;
                }
                key_device[i].pre_record=0;
            }
            
        }
        key_device[i].last_state = key_device[i].state;
    }
}   