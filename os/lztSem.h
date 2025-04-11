//信号量 计数信号量

#ifndef LZTSEM_H
#define LZTSEM_H

#include "lztConfig.h"

#if !defined(LZTOS_ENABLE_SEM) || LZTOS_ENABLE_SEM == 1

#include "lztEvent.h"

// 信号量类型
typedef struct _lztSem {
    lztEvent event;               /**< 事件控制块 */
    uint32_t count;             /**< 当前的计数 */
    uint32_t maxCount;          /**< 最大计数 */
} lztSem;

// 信号量的信息类型
typedef struct _lztSemInfo {
    uint32_t count;             /**< 当前信号量的计数 */
    uint32_t maxCount;          /**< 信号量允许的最大计 */
    uint32_t taskCount;         /**< 当前等待的任务计数 */
} lztSemInfo;

void lztSemInit (lztSem *sem, uint32_t startCount, uint32_t maxCount);
uint32_t lztSemWait (lztSem *sem, uint32_t waitTicks);
uint32_t lztSemNoWaitGet (lztSem *sem);
void lztSemNotify (lztSem *sem);
void lztSemGetInfo (lztSem *sem, lztSemInfo *info);
uint32_t tSemDestroy (lztSem *sem);

#endif

#endif /* TSEM_H */

/** @} */
