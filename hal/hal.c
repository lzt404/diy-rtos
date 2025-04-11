#include <stdio.h>
#include "lztOS.h"

#define HAL_STM32           // HAL所用的硬件平台

#define IRQ_HIGH       0
#define IRQ_LOW        1

#ifdef HAL_STM32
    #include <stm32f10x.h> 
    #include "stm32f10x_conf.h"
#endif

#include "core_cm3.h"

#include "hal.h"

#if LZTOS_ENABLE_MUTEX == 1
    lztMutex xprintfMutex;
#endif

void usartInit (void) {
#ifdef HAL_STM32
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
#endif

#if LZTOS_ENABLE_MUTEX == 1
    lztMutexInit(&xprintfMutex);
#endif
}

/*
 * 初始化中断配置
 */
void interruptInit (void) {
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    NVIC_SetPriority((IRQn_Type)IRQ_HIGH, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority((IRQn_Type)IRQ_LOW, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
}

/**
 * 进入低功耗模式
 */
void targetEnterSleep (void) {
#ifdef HAL_STM32
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
#else
    __WFI();
#endif
}

/**
 * 串口发送数据
 * @param ch
 */
void uartSendChar (char ch) {
#ifdef HAL_STM32
    USART_SendData(USART1, (unsigned char) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
#else
    if (DEMCR & TRCENA) {
        while (ITM_Port32(0) == 0);
        ITM_Port8(0) = ch;
    }
#endif
}

/**
 * 用于stm32f103 printf重定向到usart1
 */
int fputc (int ch, FILE *f) {
    uartSendChar(ch);
    return (ch);
}

/**
 * 通过延迟切换到其它任务，以模拟任务被突然强制中断的情况
 */
void interruptByOtherTask (void) {
    lztTaskDelay(1);
}

/**
 * 使能指定的中断
 * @param irq 使能的中断序号
 */
void interruptEnable (enum IRQType irq, int enable) {
    IRQn_Type type;

    switch (irq) {
        case IRQ_PRIO_HIGH:
            type = (IRQn_Type)IRQ_HIGH;
            break;
        case IRQ_PRIO_MIDDLE:
            type = (IRQn_Type)IRQ_LOW;
            break;
        case IRQ_PRIO_LOW:
            break;
    }
    if (enable) {
        NVIC_EnableIRQ(type);
    } else {
        NVIC_DisableIRQ(type);
    }
}

/**
 * 通过设置IRQ挂起位，主动触发一个中断
 */
void interruptByIRQ (enum IRQType irq) {
    IRQn_Type type;

    switch (irq) {
        case IRQ_PRIO_HIGH:
            type = (IRQn_Type)IRQ_HIGH;
            break;
        case IRQ_PRIO_MIDDLE:
            type = (IRQn_Type)IRQ_LOW;
            break;
        case IRQ_PRIO_LOW:
            break;
    }

    NVIC_SetPendingIRQ(type);
}

/**
 * 中断处理函数
 */
__weak void IRQHighHandler (void) {

}

/**
 * 中断处理函数
 */
__weak void IRQMiddleHandler (void) {

}


/**
 * 中断处理函数
 */
__weak void IRQLowHandler (void) {

}

#ifdef HAL_STM32
/**
 * 中断处理函数
 */
void WWDG_IRQHandler (void) {
    IRQHighHandler();
}

/**
 * 中断处理函数
 */
void PVD_IRQHandler (void) {
    IRQMiddleHandler();
}

#endif

/**
 *
 * @param mem
 * @param size
 */
void printMem (uint8_t *mem, uint32_t size) {
    uint32_t i;
    uint32_t column = 0;

    for (i = 0; i < size; i++) {
        if (++column % 8 == 0) {
            xprintf("0x%x\n", mem[i]);
        } else {
            xprintf("0x%x,", mem[i]);
        }
    }
    xprintf("\n");
}

/**
 * 初始化目标设备
 */
void halInit (void) {
    usartInit();
    interruptInit();
}
