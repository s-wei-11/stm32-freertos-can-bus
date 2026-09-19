#include "uart_app.h"

#include "string.h"

#include <machine/endian.h>
#include <stdint.h>
#include <stdio.h>



/*

用DMA 是因为避免receive 每接收一个字节 就打断一次cpu
而用空闲中断 是因为-用硬件来帮我断句        —— 后面不用空闲中断断句  是因为如果用了 dma需要配合改为 普通模式 造成结构空隙 容易漏收数据

最简单的方法就是用一个单一数据池来接受上位机的数据（但是这样数据容易被覆盖踩踏）
但是还有一个问题 数据突发怎么办？上位机突发5包进来 但是第一包还没处理完；那数据池要么被覆盖？要么丢失数据…
那就建造一个大的数据池子 引入一个环形缓冲区;（这样可以一定程度上应对数据突发）
但是此时又会出现一个问题 就是数据避免不了粘包；—— （只能上位机发送时加入包头包尾）
//那就每次接收时 提前自动添加 “包头”与实际数据长度 （这一行是我最开始的想法，想着利用空闲中断来手动实现加入包头包尾等
// 但是现在感觉- 到了加包头包尾这一步 那就是在设计协议  而不是在串口助手里面手动发送）

对于DMA循环模式和普通模式的理解 —— 循环模式和普通模式没什么区别 只是在数据NTDR变为0时DMA不会关闭 而是自动重置为预设值 
而对于如果加了包头包尾 然后设计了状态机解析函数 最后剥出来的数据 也是需要用载体接收 但是不同的时 dma那一步决定了 数据会不会漏收；而第二次接收
是人位决定；可以根据需要 设计单一的 状态包接收 ；或者3帧-5帧的流水包都可以


            不管开不开DMA循环 也不管用什么方法 只要生产和消费速度始终不匹配 那么怎么做都会丢失数据
            用dma循环模式也只是避免在常规模式 切换下一次dma开启时空隙漏数据
            而环形队列也只是 avoid data burst

如果用环形队列 一定要处理 head-生产 | tail-消费     
如果不用DMA 那么就用cpu轮询来检测 每进入一次中断就 head++ 然后cpu轮询处理tail++ 
如果用dma 普通模式 + idle 则 head 在进入空闲中断时进行获取NTDR  然后也还是在主循环里面消费
DMA循环模式 不处理粘包 则初始化一下后 直接在主循环里面进行轮询每次 轮询前获取NTDR计算head指针


现在要处理的问题：
1、 开启循环模式的话？ head 和 tail 两个指针怎么弄【怎么去读取数据】
2、 怎么去添加包头 以及 计算长度  {设计一个发送函数}
3、 开不开循环模式有什么区别
*/
/*
            -- DMA不论是不是循环 再NTDR结束前 都一样会处于就绪或执行中状态
            -- 数据写入肯定要读取;而每次在读取前-通过dma的NTDR获取一下位置即可      


*/


//重定向printf
int _write(int file,char *ptr , int len)
{
    HAL_UART_Transmit(huart_x, (uint8_t *)ptr,len,HAL_MAX_DELAY );
    return len;
}



/**
 * @brief   将结构体初始化 绑定外部数据池 并开启dma
 * 
 * @param obj       结构体对象
 * @param rx_pool   外部数据池
 * @param size      数据池字节数 用sizeof即可
 */
void ringbuf_init(ubuf_t * obj,uint8_t *rx_pool,uint16_t size)
{
    obj->p_head=0;
    obj->p_tail=0;
    obj->uart_buffer = rx_pool;
    obj->ubuf_size  =  size;        //算有多少字】=

    HAL_UART_Receive_DMA(huart_x,obj->uart_buffer,obj->ubuf_size);
}


/**
 * @brief         DMA循环模式获取单字节
 * 
 * @param obj 
 * @param rx_data       逐个取出字节
 * @return true         成功取出
 * @return false 
 */
bool ringbuf_pop(ubuf_t * obj,uint8_t * rx_data)
{
    obj->p_head = ( (obj->ubuf_size - (__HAL_DMA_GET_COUNTER(huart1.hdmarx)))%obj->ubuf_size ); //如果不是循环模式肯定不能在这里处理head
    if(obj->p_head != obj->p_tail)
    {
        *rx_data = obj->uart_buffer[obj->p_tail];
        obj->p_tail = (obj->p_tail+1) % obj->ubuf_size;  //向前移动指针
        return true;
    }
    return false;
}


/**
 * @brief           打包发送数据
 * 
 * @param huart     
 * @param p_data    要发送的数据
 * @param len       数据位长度
 */
void packet_send(UART_HandleTypeDef *huart, const uint8_t *p_data, uint8_t len)
{
    if (p_data == NULL) return;

    uint8_t header[3] = {0xAA, 0x55, len};
    uint8_t chksum = 0xAA + 0x55 + len;

    // 1. 发送包头与长度
    HAL_UART_Transmit(huart, header, 3, 10);

    // 2. 直接发送外部原始数据（不经过任何中间拷贝，零内存开销！）
    HAL_UART_Transmit(huart, (uint8_t *)p_data, len, 50);

    // 3. 边算校验边在最后发出
    for (uint8_t i = 0; i < len; i++) {
        chksum += p_data[i];
    }
    HAL_UART_Transmit(huart, &chksum, 1, 10);
}
 
/**
 * @brief       用于协议解析
 * @note    配合pop函数 从大数据池里面取出数据 然后进行分析
 * @param byte  要被分析的字节
 * @param out_pkt   用于接收一帧数据（去除了包头等）
 * @return true     成功
 * @return false    失败
 */
 //协议解析 是正常的；如果操作的对象需要流水包则还需要根据需求做一个 消息队列【只要生产和消费始终不匹配那么不论怎么处理都会有问题】
 //但是很多时候指令也就需要被覆盖
bool parse_byte(uint8_t byte, realData_t * out_pkt)
{
    static parse_t state = wait_head1;//初始为等待包头1
    static realData_t rx_pkt;       //用于存数值最后返回给外部
    static uint8_t data_idx = 0;
    static uint8_t chksum=0;  //用于校验

    switch (state) 
    {
        case wait_head1: 
            if(byte == packet_head1)state=wait_head2;

        ;break;

        case wait_head2: 
            if(byte == packet_head2)
            {
                state = wait_len;
                chksum = (packet_head1+packet_head2);                
            }
            else {
                state = wait_head1; //如果不对 打回去
            }
        ;break;

        case wait_len: 
            if(byte <= realData_scale)  //判断是否小于接收结构体的宽度
            {
                rx_pkt.len=byte;    //将长度存入局部静态变量
                chksum+=byte;
                data_idx=0;
                //若长度为0 跳到校验状态
                state=(rx_pkt.len==0)?wait_check:wait_data; //如果长度为0跳转到 校验状态
            }
            else {
                state = wait_head1; //打回去
            }

        ;break;

        case wait_data: 
            rx_pkt.data[data_idx++]=byte;   //存数值   idx后自增
            chksum +=byte; //校验用
            if(data_idx >= rx_pkt.len)//等于需要的长度 就停止接收 进入校验
            {
                state=wait_check;
            }
        ;break;

        case wait_check: 
            state = wait_head1; //无论成功与否 都回到起点
            if(byte == chksum)  //只有通过才可以把数据打出去
            {
                *out_pkt = rx_pkt; //校验通过
                return true;    //返回真值
            }
        ;break;
    }
    //switch里面任何一个不满足就会走到这里 返回false
    return false;
}



void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)
    {
     //  __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        /*
         * 这里只负责：
         * 1. 判断 IDLE
         * 2. 清除 IDLE
         * 3. 计算/记录新收到的数据
         * 4. 通知主循环或任务
         */
        // printf("空闲触发一次\r\n");
    }

    HAL_UART_IRQHandler(&huart1);
}

