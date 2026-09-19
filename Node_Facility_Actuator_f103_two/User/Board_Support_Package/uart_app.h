#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "usart.h"

#define huart_x &huart1




//#define ubuf_size   512
//#define ubuf_temp   128


#define realData_scale 16   //所有buf size 大小应为2的整数次幂

//包头 可自定义修改
#define packet_head1 0xaa  
#define packet_head2 0x55


//用于状态机分析状态
typedef enum{
    wait_head1=0,
    wait_head2, //第二个包头
    wait_len,   //数据长度
    wait_data,  //接收具体数据
    wait_check  //校验状态
}parse_t;


typedef struct{
   volatile uint16_t p_head;    //生产
    volatile uint16_t p_tail;
    //内存不应在里面设置死；应该由外部传入
    uint8_t  * uart_buffer;
    uint16_t ubuf_size;
} ubuf_t;


//用于接收数据
typedef struct{
    uint16_t len;
    uint8_t  data[realData_scale];
}realData_t;


//循环模式dma 不处理粘包
void ringbuf_init(ubuf_t * obj,uint8_t *rx_pool,uint16_t size);
bool ringbuf_pop(ubuf_t * obj,uint8_t * rx_data);

//循环模式 处理粘包 parse_byte时 需要传入 realData结构体用于接收数据
void packet_send(UART_HandleTypeDef *huart, const uint8_t *p_data, uint8_t len);
bool parse_byte(uint8_t byte, realData_t * out_pkt);