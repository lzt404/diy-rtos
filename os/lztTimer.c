//软定时器 软定时器

#include "lztTimer.h"
#include "lztOS.h"

#if !defined(LZTOS_ENABLE_TIMER) || LZTOS_ENABLE_TIMER == 1

static lztList lztTimerHardList;                    // "硬"定时器列表
static lztList lztTimerSoftList;                    // "软"定时器列表
static lztSem lztTimerProtectSem;                   // 用于访问软定时器列表的信号量
static lztSem lztTimerTickSem;                      // 用于软定时器任务与中断同步的计数信号量

/**
 * 初始化定时器
 * @param timer 等待初始化的定时器
 * @param delayTicks 定时器初始启动的延时ticks数。
 * @param durationTicks 给周期性定时器用的周期tick数，一次性定时器无效
 * @param timerFunc 定时器回调函数
 * @param arg 传递给定时器回调函数的参数
 * @param timerFunc 定时器回调函数
 * @param config 定时器的初始配置
 */
void lztTimerInit (lztTimer *timer, uint32_t delayTicks, uint32_t durationTicks,
                 void (*timerFunc) (void *arg), void *arg, uint32_t config) {
    lztNodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;
    timer->durationTicks = durationTicks;
    timer->timerFunc = timerFunc;
    timer->arg = arg;
    timer->config = config;

    // 如果初始启动延时为0，则使用周期值
    if (delayTicks == 0) {
        timer->delayTicks = durationTicks;
    } else {
        timer->delayTicks = timer->startDelayTicks;
    }
    timer->state = lztTimerCreated;
}

/**
 * 启动定时器
 * @param timer 等待启动的定时器
 */
void lztTimerStart (lztTimer *timer) {
    switch (timer->state) {
        case lztTimerCreated:
        case lztTimerStopped:
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
            timer->state = lztTimerStarted;

            // 根据定时器类型加入相应的定时器列表
            if (timer->config & TIMER_CONFIG_TYPE_HARD) {
                // 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
                uint32_t status = lztTaskEnterCritical();

                // 加入硬定时器列表
                lztListAddLast(&lztTimerHardList, &timer->linkNode);

                lztTaskExitCritical(status);
            } else {
                // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
                lztSemWait(&lztTimerProtectSem, 0);
                lztListAddLast(&lztTimerSoftList, &timer->linkNode);
                lztSemNotify(&lztTimerProtectSem);
            }
            break;
        default:
            break;
    }
}

/**
 * 终止定时器
 * @param timer 等待启动的定时器
 */
void lztTimerStop (lztTimer *timer) {
    switch (timer->state) {
        case lztTimerStarted:
        case lztTimerRunning:
            // 如果已经启动，判断定时器类型，然后从相应的延时列表中移除
            if (timer->config & TIMER_CONFIG_TYPE_HARD) {
                // 硬定时器，在时钟节拍中断中处理，所以使用critical来防护
                uint32_t status = lztTaskEnterCritical();

                // 从硬定时器列表中移除
                lztListRemove(&lztTimerHardList, &timer->linkNode);

                lztTaskExitCritical(status);
            } else {
                // 软定时器，先获取信号量。以处理此时定时器任务此时同时在访问软定时器列表导致的冲突问题
                lztSemWait(&lztTimerProtectSem, 0);
                lztListRemove(&lztTimerSoftList, &timer->linkNode);
                lztSemNotify(&lztTimerProtectSem);
            }
            timer->state = lztTimerStopped;
            break;
        default:
            break;
    }
}

/**
 * 销毁定时器
 * timer 销毁的定时器
 */
void lztTimerDestroy (lztTimer *timer) {
    lztTimerStop(timer);
    timer->state = lztTimerDestroyed;
}

/**
 * 查询状态信息
 * @param timer 查询的定时器
 * @param info 状态查询存储的位置
 */
void lztTimerGetInfo (lztTimer *timer, lztTimerInfo *info) {
    uint32_t status = lztTaskEnterCritical();

    info->startDelayTicks = timer->startDelayTicks;
    info->durationTicks = timer->durationTicks;
    info->timerFunc = timer->timerFunc;
    info->arg = timer->arg;
    info->config = timer->config;
    info->state = timer->state;

    lztTaskExitCritical(status);
}

/**
 * 遍历指定的定时器列表，调用各个定时器处理函数
 */
static void lztTimerCallFuncList (lztList *timerList) {
    lztNode *node;
		uint32_t count;

		for (node = lztListFirst(timerList), count = lztListCount(timerList); count > 0; count--) {
    // 检查所有任务的delayTicks数，如果不0的话，减1。
        lztTimer *timer = lztNodeParent(node, lztTimer, linkNode);

        // 如果延时已到，则调用定时器处理函数
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0)) {
            // 切换为正在运行状态
            timer->state = lztTimerRunning;

            // 调用定时器处理函数
            timer->timerFunc(timer->arg);

            // 切换为已经启动状态
            timer->state = lztTimerStarted;

            if (timer->durationTicks > 0) {
                // 如果是周期性的，则重复延时计数值
                timer->delayTicks = timer->durationTicks;
            } else {
                // 否则，是一次性计数器，中止定时器
                lztListRemove(timerList, &timer->linkNode);
                timer->state = lztTimerStopped;
            }
        }
				
				node = lztListNext(timerList, node);
    }
}

/**
 * 处理软定时器列表的任务
 */
static lztTask lztTimeTask;
static lztTaskStack lztTimerTaskStack[LZTOS_TIMERTASK_STACK_SIZE];

static void lztTimerSoftTask (void *param) {
    for (;;) {
        // 等待系统节拍发送的中断事件信号
        lztSemWait(&lztTimerTickSem, 0);

        // 获取软定时器列表的访问权限
        lztSemWait(&lztTimerProtectSem, 0);

        // 处理软定时器列表
        lztTimerCallFuncList(&lztTimerSoftList);

        // 释放定时器列表访问权限
        lztSemNotify(&lztTimerProtectSem);
    }
}

/**
 * 通知定时模块，系统节拍tick增加
 */
void lztTimerModuleTickNotify (void) {
    uint32_t status = lztTaskEnterCritical();

    // 处理硬定时器列表
    lztTimerCallFuncList(&lztTimerHardList);

    lztTaskExitCritical(status);

    // 通知软定时器节拍变化
    lztSemNotify(&lztTimerTickSem);
}

/**
 * 定时器模块初始化
 */
void lztTimerModuleInit (void) {
    lztListInit(&lztTimerHardList);
    lztListInit(&lztTimerSoftList);
    lztSemInit(&lztTimerProtectSem, 1, 1);
    lztSemInit(&lztTimerTickSem, 0, 0);
}

/**
 * 初始化软定时器任务
 */
void lztTimerInitTask (void) {

#if LZTOS_TIMERTASK_PRIO >= (LZTOS_PRO_COUNT - 1)
#error "The proprity of timer task must be greater then (lztOS_PRO_COUNT - 1)"
#endif
    lztTaskInit(&lztTimeTask, lztTimerSoftTask, (void *) 0,
              LZTOS_TIMERTASK_PRIO, lztTimerTaskStack, sizeof(lztTimerTaskStack));

}

lztTask * lztTimerTask (void) {
    return &lztTimeTask;
}

#endif
