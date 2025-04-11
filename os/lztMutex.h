//互斥信号量 互斥信号量

#ifndef LZTMUTEX_H
#define LZTMUTEX_H

#include "lztConfig.h"

#if !defined(LZTOS_ENABLE_MUTEX) || LZTOS_ENABLE_MUTEX == 1

#include "lztEvent.h"

// 互斥信号量类型
typedef struct _lztMutex {
    lztEvent event;                   /**< 事件控制块 */

    uint32_t lockedCount;           /**< 已被锁定的次数 */
    lztTask *owner;                   /**< 拥有者 */
    uint32_t ownerOriginalPrio;     /**< 拥有者原始的优先级 */
} lztMutex;

// 互斥信号量查询结构
typedef struct _tMutexInfo {
    uint32_t taskCount;             /**< 等待的任务数量 */

    uint32_t ownerPrio;             /**< 拥有者任务的优先级 */
    uint32_t inheritedPrio;         /**< 继承优先级 */

    lztTask *owner;                   /**< 当前信号量的拥有者 */
    uint32_t lockedCount;           /**< 锁定次数 */
} lztMutexInfo;

void lztMutexInit (lztMutex *mutex);
uint32_t lztMutexWait (lztMutex *mutex, uint32_t waitTicks);
uint32_t lztMutexNoWaitGet (lztMutex *mutex);
uint32_t lztMutexNotify (lztMutex *mutex);
uint32_t lztMutexDestroy (lztMutex *mutex);
void lztMutexGetInfo (lztMutex *mutex, lztMutexInfo *info);

#endif 

#endif /* TMUTEX_H */

/** @} */
