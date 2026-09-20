#include "Delay.h"

#include "stm32f1xx.h"


//初始化
void DWT_init(void)
{
    CoreDebug->DEMCR |=CoreDebug_DEMCR_TRCENA_Msk;  //  开启跟踪系统全局开关    demcr为调试异常监视寄存器   trcena为跟踪使能位
    DWT->CYCCNT=0;                                  //计数器清零
    DWT->CTRL |=DWT_CTRL_CYCCNTENA_Msk;             //开启计数器 硬件自增
}

void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;   //记录开始的时候
    uint32_t    ticks = us* (SystemCoreClock / 1000000);    //系统时钟单位为S 如80mhz 80 000 000除完之后为 1us80个时钟周期
    //ticks 最后算出的是需要多少个 hz
    //cyccnt 为时钟周期计数器-寄存器 然后减去开始的 当差额达到 指定的us 也就是算出的hz个数 停止阻塞
    while((DWT->CYCCNT - start) < ticks);
}
