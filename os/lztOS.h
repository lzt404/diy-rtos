//核心头文件

#ifndef LZTOS_H
#define LZTOS_H

#include <stdint.h>
#include "lztLib.h"
#include "lztConfig.h"
#include "lztTask.h"
#include "lztEvent.h"
#include "lztSem.h"
#include "lztMBox.h"
#include "lztMemBlock.h"
#include "lztFlagGroup.h"
#include "lztMutex.h"
#include "lztTimer.h"
#include "lztHooks.h"

#define TICKS_PER_SEC                   (1000 / LZTOS_SYSTICK_MS)

// 错误码
typedef enum _tError {
    lztErrorNoError = 0,                                  /**< 没有错误 */
    lztErrorTimeout,                                      /**< 等待超时 */
    lztErrorResourceUnavaliable,                          /**< 资源不可用 */
    lztErrorDel,                                          /**< 被删除 */
    lztErrorResourceFull,                                 /**< 资源缓冲区满 */
    lztErrorOwner,                                        /**< 不匹配的所有者 */
} lztError;

//! 当前任务：记录当前是哪个任务正在运行
extern lztTask *currentTask;

//! 下一个将即运行的任务：在进行任务切换前，先设置好该值，然后任务切换过程中会从中读取下一任务信息
extern lztTask *nextTask;

typedef uint32_t lztTaskCritical_t;

lztTaskCritical_t lztTaskEnterCritical (void);
void lztTaskExitCritical (lztTaskCritical_t status);

void lztTaskRunFirst (void);

void lztTaskSwitch (void);
lztTask *lztTaskHighestReady (void);

void lztTaskSchedInit (void);
void lztTaskSchedDisable (void);
void lztTaskSchedEnable (void);
void lztTaskSchedRdy (lztTask *task);
void lztTaskSchedUnRdy (lztTask *task);
void lztTaskSchedRemove (lztTask *task);
void lztTaskSched (void);
		 
void lztTimeTaskWait (lztTask *task, uint32_t ticks);
void lztTimeTaskWakeUp (lztTask *task);
void lztTimeTaskRemove (lztTask *task);
void lztTaskDelay (uint32_t delay);
void lztTaskSystemTickHandler (void);
		 
void lztInitApp (void);
void lztSetSysTickPeriod (uint32_t ms);
float lztCpuUsageGet (void);
lztTask * lztIdleTask (void);

#endif
