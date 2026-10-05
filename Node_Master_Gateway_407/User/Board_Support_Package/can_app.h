#pragma once
#include "can.h"
#include "stdbool.h"
#define buff_scale 16       //缓冲区大小

typedef struct{
    CAN_RxHeaderTypeDef C_id_t;    //存id 以及dlc rtr等报文信息
    uint8_t  Data[8];
}Can_data;

typedef struct{
    volatile uint16_t head_t;    //头指针写入用 让中断操作
    volatile uint16_t tail_t;    //未指针读取用 主循环用
    Can_data C_buff[buff_scale];
}Can_buff_t;

/*---------------------- 入栈/出栈 — 函数--------------- */
void Can_buff_init(Can_buff_t * obj);
bool Can_buff_pop(Can_buff_t * obj,Can_data * out_msg);
bool Can_buff_push(CAN_HandleTypeDef *hcan, uint32_t fifo_num,Can_buff_t * obj);


/*——————————————————— CAN通信  API ————————————————————*/
bool Can_Init(CAN_HandleTypeDef *hcan);
uint8_t CAN1_Send_Test(void);
bool Can_TxStdData(CAN_HandleTypeDef *hcan, uint16_t Tx_ID, const uint8_t *pTxData, uint8_t canTx_scale);



