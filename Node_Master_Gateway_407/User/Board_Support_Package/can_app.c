#include "can_app.h"
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_def.h"
#include <stdbool.h>
#include <stdint.h>



/**
 * @brief      初始化过滤器 以及中断 并开始工作
 * 
 * @param hcan 
 * @return true     工作正常
 * @return false 
 */
bool Can_Init(CAN_HandleTypeDef *hcan)
{
     CAN_FilterTypeDef filter_zero;
     filter_zero.FilterActivation=CAN_FILTER_ENABLE ;  //开启过滤器
     filter_zero.FilterBank   = 0; //指定编号
     filter_zero.SlaveStartFilterBank = 14;  //103只有14个
     filter_zero.FilterFIFOAssignment = 0; //进入fifo0
     //全通模式
     filter_zero.FilterIdHigh = 0x001<<5; //id1         //[ 0000-0000-0001 00000]
     filter_zero.FilterIdLow =0x000<<5;   //id2

     //只要 id1 且为标准数据帧
     filter_zero.FilterMaskIdHigh = (0x7ff<<5)|(0x10); //id1掩码   //[ 1111 1111 1111 | 1 0 000]0x10
     filter_zero.FilterMaskIdLow = 0xffff;  //id2掩码
     filter_zero.FilterMode = CAN_FILTERMODE_IDMASK;
     filter_zero.FilterScale = CAN_FILTERSCALE_16BIT;   //16位
     if(HAL_CAN_ConfigFilter(hcan,&filter_zero) != HAL_OK)goto error;

     //使能我需要的中断
     if(HAL_CAN_ActivateNotification(hcan,CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)goto error ;

     //进入工作模式
     if(HAL_CAN_Start(hcan) != HAL_OK)goto error;
    
     return true;

error:
     return false;
     //Error_Handler();
}






/**
 * @brief  CAN1 环回模式下 发送测试报文
 * @retval 0: 成功写入发送邮箱; 1: 邮箱全满或发送失败
 */
uint8_t CAN1_Send_Test(void)
{
    CAN_TxHeaderTypeDef TxHeader;           // 发送报文协议头结构体
    // 待发送的 8 字节数据载荷
    uint8_t TxData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};                
    uint32_t TxMailbox;                     // 接收函数返回的实际占用的发送邮箱号（0/1/2）

    TxHeader.StdId = 0x005;                 // 标准 ID（11位宽，取值范围 0x000 ~ 0x7FF）
    TxHeader.ExtId = 0x00;                  // 扩展 ID（29位宽，本帧为标准帧，此项填 0）
    TxHeader.IDE   = CAN_ID_STD;            // 帧类型标识：标准帧（CAN_ID_EXT 为扩展帧）
    TxHeader.RTR   = CAN_RTR_DATA;          // 帧属性：数据帧（CAN_RTR_REMOTE 为远程请求帧）
    TxHeader.DLC   = 8;                     // 数据长度：8 个字节（取值范围 0 ~ 8）
    TxHeader.TransmitGlobalTime = DISABLE;  // 关闭时间戳捕获功能（通常用于 TTCM 模式）

    // 查询硬件 3 个发送邮箱，将数据写入空闲邮箱并置位发送请求（TXRQ）
    // 注意：此函数为非阻塞，返回 HAL_OK 仅表示入队成功，硬件随后自动抢占总线仲裁发送
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        return 1; // 3 个发送邮箱全满，入队失败
    }
    
    return 0;     // 成功推入发送邮箱
}



/**
 * @brief  CAN 发送标准数据帧（11位 ID）
 * @param  hcan         CAN 外设句柄指针
 * @param  Tx_ID        标准帧 ID（0x000 ~ 0x7FF）
 * @param  pTxData      待发送的数据缓冲区指针
 * @param  canTx_scale  数据长度（0 ~ 8）
 * @return true: 写入发送邮箱成功; false: 邮箱满或发送失败
 */
bool Can_TxStdData(CAN_HandleTypeDef *hcan, uint16_t Tx_ID, const uint8_t *pTxData, uint8_t canTx_scale)
{
    CAN_TxHeaderTypeDef TxData_type = {0}; // 结构体清零，防止栈上垃圾值
    uint32_t Txmail_id = 0;                // 保存硬件分配的发送邮箱号（0/1/2） ………可以不用但是不能没有

    // 1. 安全检查：参数合法性与发送邮箱判空
    if (hcan == NULL || canTx_scale > 8 || HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
    {
        return false;
    }

    // 2. 填充协议头
    TxData_type.StdId = Tx_ID;                 // 11位标准 ID
    TxData_type.IDE   = CAN_ID_STD;            // 标准帧
    TxData_type.RTR   = CAN_RTR_DATA;          // 数据帧
    TxData_type.DLC   = canTx_scale;           // 数据长度
    TxData_type.TransmitGlobalTime = DISABLE;  // 关闭时间戳

    // 3. 推入空闲发送邮箱
    if (HAL_CAN_AddTxMessage(hcan, &TxData_type, (uint8_t *)pTxData, &Txmail_id) != HAL_OK)
    {
        return false;
    }

    return true;
}

/**
 * @brief  CAN 发送扩展数据帧（29位 ID）
 * @param  hcan         CAN 外设句柄指针
 * @param  Tx_ID        扩展帧 ID（0x00000000 ~ 0x1FFFFFFF）
 * @param  pTxData      待发送的数据缓冲区指针
 * @param  canTx_scale  数据长度（0 ~ 8）
 * @return true: 写入发送邮箱成功; false: 邮箱满或发送失败
 */
bool Can_TxExtData(CAN_HandleTypeDef *hcan, uint32_t Tx_ID, const uint8_t *pTxData, uint8_t canTx_scale)
{
    CAN_TxHeaderTypeDef TxData_type = {0};
    uint32_t Txmail_id = 0;        //接收当前数据帧在哪个邮箱编号

    // 1. 安全检查：参数合法性与发送邮箱判空
    if (hcan == NULL || canTx_scale > 8 || HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0)
    {
        return false;
    }

    // 2. 填充协议头
    TxData_type.ExtId = Tx_ID;                 // 29位扩展 ID
    TxData_type.IDE   = CAN_ID_EXT;            // 扩展帧
    TxData_type.RTR   = CAN_RTR_DATA;          // 数据帧
    TxData_type.DLC   = canTx_scale;           // 数据长度
    TxData_type.TransmitGlobalTime = DISABLE;  // 关闭时间戳

    // 3. 推入空闲发送邮箱
    if (HAL_CAN_AddTxMessage(hcan, &TxData_type, (uint8_t *)pTxData, &Txmail_id) != HAL_OK)
    {
        return false;
    }

    return true;
}



void Can_buff_init(Can_buff_t * obj)
{
    //初始指针都为0
    obj->head_t=0;
    obj->tail_t=0;
}


/**
 * @brief 弹出具体的一帧数据
 * 
 * @param obj       具体的can缓冲区对象
 * @param out_msg   ⭐填入要接受一帧数据的接收容器
 * @return true      接收成功
 * @return false       接收失败——数据池为空
 */
bool Can_buff_pop(Can_buff_t * obj,Can_data * out_msg)
{
    if(obj->head_t == obj->tail_t)  //不相等说明数据池不为空
    {
        return false;
    }
    * out_msg = obj->C_buff[obj->tail_t];   //把具体的一帧数据传出去
    obj->tail_t=(obj->tail_t+1)%buff_scale; //循环移动下标
    return true;
}


char can_rx[8]; //临时暂存实验用
/**
 * @brief           将数据压入栈
 * 
 * @param hcan     具体的can外设
 * @param fifo_num 哪一个fifo
 * @param obj      缓冲区对象
 * @return true    入栈成功
 * @return false   入栈失败
 */
bool Can_buff_push(CAN_HandleTypeDef *hcan, uint32_t fifo_num,Can_buff_t * obj)
{
    CAN_RxHeaderTypeDef RxHeader; // 存放接收到的协议头（ID、DLC、帧类型等）
    uint8_t RxData[8];      // 存放接收到的 8 字节数据载荷
    if(HAL_CAN_GetRxMessage(hcan,fifo_num, &RxHeader, RxData)==HAL_OK)
    {
        if((obj->head_t+1)%buff_scale != obj->tail_t)     //说明还有空间
        {
            obj->C_buff[obj->head_t].C_id_t=RxHeader; //存数据的具体信息
            for(uint8_t i=0;i<8;i++)
            {
                obj->C_buff[obj->head_t].Data[i]=RxData[i];
                can_rx[i]=RxData[i];
            }
            obj->head_t=(obj->head_t+1)%buff_scale;
            return true;
        }
    }
    return false;
}



