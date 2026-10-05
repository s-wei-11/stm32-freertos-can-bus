#include "key.h"






void key_attach_callback(key_t * dev,key_callback  handler_func)
{
    dev->callback=handler_func; //绑定业务处理函数
}


static inline void notify_event(key_t * dev,key_state event)
{
    if(dev->callback!=NULL && event!=key_none)
    {
        dev->callback(dev,event);   //传入当前设备啊，以及触发的事件
    }
}

void key_scan(key_t * key,uint8_t key_num)  //传入按键对象 以及按键数量
{
    uint32_t now_time=HAL_GetTick();
    for(uint8_t i=0;i<key_num;i++)
    {
        key[i].key_state=HAL_GPIO_ReadPin(key[i].port, key[i].pin); //循环获取对应引脚的状态


        if(key[i].key_state==0 && key[i].key_last_state==1)
        {
            if(now_time-key[i].press_time>=20 || key[i].press_time==0)  //扫描放入10-20ms循环的话 这步判断可有可无
            {
                key[i].press_time=now_time;
                key[i].record++;
                key[i].key_lock=1;  //解长按锁
             }
            
        }
        else if(key[i].key_state==0 && key[i].key_last_state==0)
        {
            if(now_time-key[i].press_time>1500 && key[i].key_lock==1)
            {
                key[i].record=0;    //按键次数归零
                key[i].key_lock=0;  //上锁

                //长按状态返回
                notify_event(&key[i], key_long_press);
            }
        }
        else if(key[i].key_state==1 && key[i].key_last_state==0)    //上升
        {
            key[i].release_time=now_time;   //记录按键抬起（松手）时间
            
            /*
            if(key[i].release_time-key[i].press_time>500 && key[i].record==1)   
            {
                //返回短按状态
                key[i].record=0;//按键次数归零
            }
            */
        }
        else if(key[i].key_state==1 && key[i].key_last_state==1)    //全部结束 清算连击
        {
            if(key[i].record>0 && now_time-key[i].release_time>250) //进入连击结算
            {
                switch(key[i].record)
                { 
                    case 1:notify_event(&key[i], key_single_click);;break;  //单击
                    case 2:notify_event(&key[i], key_double_click);;break;  //双击
                    case 3:notify_event(&key[i], key_triple_click);;break;  //三击
                    //default:;
                }
                key[i].record=0;
            }
        }
        key[i].key_last_state=key[i].key_state;         //更新上一次按键状态
        notify_event(&key[i], key_none); 
    }
    
}
