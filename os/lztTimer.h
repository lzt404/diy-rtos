//软定时器 软定时器

#ifndef LZTIMER_H
#define LZTIMER_H

#include "lztConfig.h"

#if !defined(LZTOS_ENABLE_TIMER) || LZTOS_ENABLE_TIMER == 1

#include "lztEvent.h"

// 定时器状态
typedef enum _lztTimerState {
    lztTimerCreated,          /**< 定时器已经创建 */
    lztTimerStarted,          /**< 定时器已经启动 */
    lztTimerRunning,          /**< 定时器正在执行回调函数 */
    lztTimerStopped,          /**< 定时器已经停止 */
    lztTimerDestroyed         /**< 定时器已经销毁 */
} lztTimerState;

// 软定时器结构
typedef struct _lztTimer {
    lztNode linkNode;                     /**< 链表结点 */

    uint32_t startDelayTicks;           /**< 初次启动延后的ticks数 */
    uint32_t durationTicks;             /**< 周期定时时的周期tick数 */
    uint32_t delayTicks;                /**< 当前定时递减计数值 */

    void (*timerFunc) (void *arg);      /**< 定时回调函数 */
    void *arg;                          /**< 传递给回调函数的参数 */

    uint32_t config;                    /**< 定时器配置参数 */
    lztTimerState state;                  /**< 定时器状态 */
} lztTimer;

// 软定时器状态信息
typedef struct _lztTimerInfo {
    uint32_t startDelayTicks;           /**< 初次启动延后的ticks数 */
    uint32_t durationTicks;             /**< 周期定时时的周期tick数 */

    void (*timerFunc) (void *arg);      /**< 定时回调函数 */
    void *arg;                          /**< 传递给回调函数的参数 */

    uint32_t config;                    /**< 定时器配置参数 */
    lztTimerState state;                  /**< 定时器状态 */
} lztTimerInfo;

//!< 软硬定时器
#define TIMER_CONFIG_TYPE_HARD          (1 << 0)
#define TIMER_CONFIG_TYPE_SOFT          (0 << 0)

void lztTimerInit (lztTimer *timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void *arg), void *arg, uint32_t config);
void lztTimerStart (lztTimer *timer);

void lztTimerStop (lztTimer *timer);

void lztTimerDestroy (lztTimer *timer);
void lztTimerGetInfo (lztTimer *timer, lztTimerInfo *info);
void lztTimerModuleTickNotify (void);
void lztTimerModuleInit (void);
void lztTimerInitTask (void);
lztTask * lztTimerTask (void);

#endif


#endif
