#include "ecode.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

//由于初始化时 默认弱上拉
/*
static volatile uint8_t pin_a = 1;
static volatile uint8_t pin_a_last_state = 1;
static volatile uint8_t pin_b = 1;
static volatile uint8_t pin_b_last_state = 1;   //这样的话不利于多个设备复用 因为多个设备将会 多次调用同一个变量
static volatile uint8_t pin_d = 1;
static volatile uint8_t pin_d_last_state = 1;
*/

/**
 * @brief        注册回调函数（也就是初始化ecode回调函数）
 *               将具体执行业务函数绑定到ecode_dev结构体中
 *
 * @param dev                设备结构体
 * @param callback           执行业务的函数 ，必须符合回调变量的参数
 * @note                        业务函数定义 需要参考回调函数规格
 */
void register_ecode_callback(ecode_dev * dev,ecode_callback callback)
{
    dev->callback=callback;
}


/**
 * @brief        判空，防止被乱调用，只有状态不为空时才  
 *                  
 *
 * @param dev       设备结构体
 * @param event     传入事件，等于是将事件传入到业务函数
 */
static inline void ecode_trigger_event(ecode_dev * dev,ecode_state event)
{
    if(dev->callback!=NULL && event!=none)  //如果业务函数已经绑定 且状态不为空则触发回调
    {
        //经历过注册函数初始化后 这里的 dev->callback 等于是具体的函数
        dev->callback(dev,event);
    }
}

/**
 * @brief 初始化传入引脚进行绑定
 * 
 */
void ecode_init(ecode_dev * dev,GPIO_TypeDef * port_a,uint16_t pin_a,GPIO_TypeDef * port_b,\
                uint16_t pin_b,GPIO_TypeDef * port_d,uint16_t pin_d)
{
    dev->port_a = port_a;
    dev->port_b = port_b;
    dev->port_d = port_d;
    dev->pin_a = pin_a;
    dev->pin_b = pin_b;
    dev->pin_d = pin_d;

    dev->pin_a_last=1;
    dev->pin_b_last=1;
    dev->pin_d_last=1;  //在这里结构体被赋值后 数据不存在栈里

    dev->lock_long=1;   //等于1时锁住
    dev->press_time=0; //按下时间，初始化
    dev->release_time=0;//松手时间，初始化
    dev->record=0;  //连按记录，初始化

    dev->count=0;  //计数器初始为0
    dev->speed=0; //最开始都为0
    dev->turn_time=0;

    dev->callback=NULL;//回调状态初始化为空
}




#define read_A(x) (!!((x->port_a)->IDR & (x->pin_a)))
#define read_B(x) (!!((x->port_b)->IDR & (x->pin_b)))
#define read_D(x) (!!((x->port_d)->IDR & (x->pin_d)))
ecode_state ecode_getstate(ecode_dev * dev)
{
    ecode_state sta=none;  //默认无操作状态
    uint8_t pin_a=read_A(dev);
    uint8_t pin_b=read_B(dev);
    uint8_t pin_d=read_D(dev);
    static  uint32_t now_time=0;    //通过中断来记录时间 与时基解耦
    now_time +=2;
    //  uint32_t now_time=HAL_GetTick();    //进入时统一获取当下时间
    //  TickType_t now_tick =xTaskGetTickCountFromISR();    //进入时统一获取当下时间
    //  uint32_t now_time = now_tick * portTICK_PERIOD_MS;

    if(pin_a == 0 && dev->pin_a_last == 1)  //A相产生下降沿  拧动时才会触发内部
    {
        if(dev->turn_time!=0)   //避免第一次转动时 计算速度会出错
        {
            uint32_t tmp_time=now_time-dev->turn_time;
            if(tmp_time!=0)dev->speed=1000/tmp_time;   //因为时间单位为ms 分子为1000
        }
        dev->turn_time=now_time;  //记录第当前转动时间
        
        if(pin_b==1)  //如此则为正转 顺时针
        {
            dev->count++;
            sta=cw;
        }
        else {
            dev->count--;
            sta=ccw;
        }
    }
    if(now_time-dev->turn_time>=1000 && dev->turn_time!=0)    //如果超过1000ms没有拧动 则将速度置零
    {
            dev->speed=0;
            dev->turn_time=0;
    }

   if(pin_d==0 && dev->pin_d_last==1)   //被按下
    {
        //人手速永远达不到让他在2ms内释放 所以nowtime始终会被刷新
        //只要满足“距离上次有效按下已过 25ms（过滤物理抖动）”或“属于开机首次按压”，即判定本次按压有效
        if(now_time-dev->press_time>=25 || dev->press_time==0)
        {
            dev->lock_long=0;//长按解锁
            dev->press_time=now_time;  //记录按下时间
            dev->record+=1; //每按一次加1
        }

    }
    else if(pin_d==0 && dev->pin_d_last==0)     //0 0的状态会有持续的情况所以做了锁 防止一直触发
    {
        if(now_time-dev->press_time>= lpress_time && dev->lock_long==0)  //如果按下时间超过1500ms
        {
            dev->lock_long=1;  //锁定
            dev->release_time=now_time;  //记录释放时间
            sta=long_press;
            dev->record=0;
        }
    }
    else if(pin_d==1 && dev->pin_d_last==0) //上升沿
    {      
        //这里不需要短按逻辑        
        /* if(now_time - dev->press_time >= short_press && now_time- dev->press_time < lpress_time) //短按
        {
            sta=short_press;
            dev->record=0;//将连击记录清0
        } */
        dev->release_time=now_time;  //记录释放时间
    }
    else if(pin_d==1 && dev->pin_d_last==1) //完全松手时
    {
        if( now_time - dev->release_time >= multi_click_t && dev->record>0) //如果释放时间超过250ms且记录数大于0 才判断
        {                                                              // 且是当前时间减去用上一次松手时间，而不是松手减按下
            if(dev->record==1)                                         // 要是在250内按下了下一次 则不进行任何操作 进行累计连击记录
            {
                sta=single_click;
            }
            else if(dev->record==2)
            {
                sta=double_click;
            }
            else if(dev->record==3)
            {
                sta=triple_click;
            }
            dev->record=0;
        }
        
    } 
    dev->pin_a_last=pin_a;
    dev->pin_b_last=pin_b;
    dev->pin_d_last=pin_d;
    //触发业务函数
    ecode_trigger_event(dev,sta);
    return sta;
}
