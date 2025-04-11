#ifndef PROJECT_TARGET_H
#define PROJECT_TARGET_H

#include "lztOS.h"
#include <stdio.h>

enum IRQType {
    IRQ_PRIO_HIGH,                  // 高优先级中断
    IRQ_PRIO_MIDDLE,                // 中优先级中断
    IRQ_PRIO_LOW,                   // 低优先级中断
};

#if LZTOS_ENABLE_MUTEX == 1  
    extern lztMutex xprintfMutex;

    #define xprintf(fmt, ...) {     \
            lztMutexWait(&xprintfMutex, 0);   \
            printf(fmt, ##__VA_ARGS__);       \
            lztMutexNotify(&xprintfMutex);    \
    }
#else
    #define xprintf(fmt, ...) { printf(fmt, ##__VA_ARGS__); }
#endif


void halInit (void);

void targetEnterSleep (void);

void interruptByOtherTask (void);

void interruptEnable (enum IRQType irq, int enable);

void interruptByIRQ (enum IRQType irq);

void xprintfMem (uint8_t * mem, uint32_t size);

__weak void IRQHighHandler (void);
__weak void IRQMiddleHandler (void);
__weak void IRQLowHandler (void);

#endif //PROJECT_TARGET_H
