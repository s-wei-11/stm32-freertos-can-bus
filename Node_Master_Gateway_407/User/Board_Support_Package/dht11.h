#ifndef __dht11_H_
#define __dht11_H_
#include "gpio.h"
#include <stdint.h>



typedef enum {
    dht11_tem=0,
    dht11_hum=1
}data_type;


typedef struct {
    // 1. 硬件资源接口 (Hardware)
    GPIO_TypeDef* portx;     // 例如: GPIOB
    uint16_t      pinx;      // 例如: GPIO_PIN_12

    // 2. 传感器解析后的结果 (Data)
    int16_t       temperature; // 放大100倍后的温度 (比如 2550 表示 25.5℃)
    int16_t       humidity;    // 放大100倍后的湿度 (比如 5000 表示 50.0%)

    // 3. 运行状态 (Status)
    uint8_t       is_valid;    // 1: 上次读取成功; 0: 读取失败
} DHT11_t;



//x需要为结构体对象
#define dht_pin_high(x) ((x->portx)->BSRR=x->pinx)
#define dht_pin_low(x)  ((x->portx)->BSRR=(uint32_t)(x->pinx) << 16U)
#define dht_read(x)     (!!((x->portx)->IDR & x->pinx))


void DHT11_Init(DHT11_t *dev, GPIO_TypeDef *portx, uint16_t pinx);
uint8_t dht_getvalue(DHT11_t * dh);

#endif