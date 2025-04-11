//事件控制块

#ifndef LZTEVENT_H
#define LZTEVENT_H

#include "lztConfig.h"
#include "lztLib.h"
#include "lztTask.h"

// Event类型
typedef enum _lztEventType {
    lztEventTypeUnknown = (0 << 16),                  /**< 未知类型 */
    lztEventTypeSem = (1 << 16),                      /**< 信号量类型 */
    lztEventTypeMbox = (2 << 16),                     /**< 邮箱类型 */
    lztEventTypeMemBlock = (3 << 16),                 /**< 存储块类型 */
    lztEventTypeFlagGroup = (4 << 16),                /**< 事件标志组 */
    lztEventTypeMutex = (5 << 16),                    /**< 互斥信号量类型 */
} lztEventType;

// Event控制结构
typedef struct _lztEvent {
    lztEventType type;                                /**< Event类型 */
    lztList waitList;                                 /**< 任务等待列表 */
} lztEvent;

void lztEventInit (lztEvent *event, lztEventType type);
void lztEventWait (lztEvent *event, lztTask *task, void *msg, uint32_t state, uint32_t timeout);
lztTask *lztEventWakeUp (lztEvent *event, void *msg, uint32_t result);
void lztEventWakeUpTask (lztEvent *event, lztTask *task, void *msg, uint32_t result);
void lztEventRemoveTask (lztTask *task, void *msg, uint32_t result);
uint32_t lztEventRemoveAll (lztEvent *event, void *msg, uint32_t result);
uint32_t lztEventWaitCount (lztEvent *event);

#endif /* TEVENT_H */

/** @} */
