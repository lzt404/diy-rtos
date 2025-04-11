#include "lztOS.h"
#include "stm32f10x.h"

/**
 * 系统时钟节拍定时器System Tick配置
 * 模拟器中，系统时钟节拍为12MHz
 * 务必按照本教程推荐配置，否则systemTick的值就会有变化，需要查看数据手册才了解
 */
void lztSetSysTickPeriod (uint32_t ms) {
    SystemCoreClockUpdate();
    SysTick->LOAD = (uint64_t)ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

/**
 * SystemTick的中断处理函数
 */
void SysTick_Handler () {
    lztTaskSystemTickHandler();
}

