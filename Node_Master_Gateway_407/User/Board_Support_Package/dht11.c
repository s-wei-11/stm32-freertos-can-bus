#include "dht11.h"
#include "delay.h"
#include "stm32f4xx_hal.h"



#include <stdint.h>



/**
 * @brief dht11硬件资源绑定 给结构体变量赋值
 *        也就是为每一个dht11指定通通
 * @param dev   dht11的结构体变量
 * @param portx dht11那个端口
 * @param pinx  dht11哪个引脚
 * @note  参数之外的成员也赋值了 防止野值
 */
void DHT11_Init(DHT11_t *dev, GPIO_TypeDef *portx, uint16_t pinx)
{
    dev->portx=portx;
    dev->pinx=pinx;
    dev->humidity=0;
    dev->temperature=0;
    dev->is_valid=0;
    
    HAL_GPIO_WritePin(dev->portx, dev->pinx, GPIO_PIN_SET);
    HAL_Delay(1000);//初始上电 暂停1s
}

uint8_t dht_getvalue(DHT11_t * dh)
{
    uint8_t dh_data_buf[5]={0};     //缓存中间值 每次刷新
    uint16_t check=0;
    uint8_t tim=0;
    //初始化时拉高过
    dht_pin_low(dh);
    HAL_Delay(19);
    dht_pin_high(dh);
    while(dht_read(dh)){delay_us(1);if(tim++>80)goto error;}//等待释放结束 直到进入从机第一个低电平应答信号
    tim=0;

    __disable_irq();
    while(!dht_read(dh)){delay_us(1);if(tim++>80)goto error;}//等待低电平应答信号结束
    tim=0;
    while(dht_read(dh)){delay_us(1);if(tim++>80)goto error;}   //等待高电平结束
    tim=0;
    for (uint8_t i=0; i<40; i++) {
        while(!dht_read(dh)){delay_us(1);if(tim++>80)goto error;}   //等待低电平过去 并校准时序
        tim=0;
        delay_us(30);
        dh_data_buf[i/8] |=(dht_read(dh)<<(7-(i%8)));
        if (dht_read(dh)) //若为1                                                                                                                                                                                                                                                                      
        {
            while(dht_read(dh)){delay_us(1);if(tim++>80)goto error;}   //等待高电平结束
            tim=0;
        }
    }
    while(!dht_read(dh)){delay_us(1);if(tim++>80)goto error;}   //等待最后一个长低电平——结束信号过去
    
    __enable_irq();
    check=dh_data_buf[0]+dh_data_buf[1]+dh_data_buf[2]+dh_data_buf[3];
    if (check==dh_data_buf[4])
    {
        //数值正确 
        dh->is_valid=1;
        dh->humidity=dh_data_buf[0]*100+dh_data_buf[1];//数值放大100倍
        if(!!(dh_data_buf[3]&0x80))
        {
            //此时温度为负数
            dh->temperature=-(dh_data_buf[2]*100+(dh_data_buf[3]&(~0x80)));
        }
        else {
            dh->temperature=(dh_data_buf[2]*100+dh_data_buf[3]);
        }
        return 1;
    }
    else {
        dh->is_valid=0;
        return 0;
    }


error:
    __enable_irq();
    dh->is_valid=0;//读取失败
    return 0;
}

