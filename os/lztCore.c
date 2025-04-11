#include "lztOS.h"
#include "stm32f10x.h"

lztTask *currentTask;                             /**< 当前任务：记录当前是哪个任务正在运行 */
lztTask *nextTask;                                /**< 下一个将即运行的任务： */
lztTask *idleTask;                                /**< 空闲任务 */

lztBitmap taskPrioBitmap;                         /**< 任务优先级的标记位置结构 */
lztList taskTable[LZTOS_PRO_COUNT];              /**< 所有任务的指针数组 */

uint8_t schedLockCount;                         /**< 调度锁计数器 */
uint32_t tickCount;                             /**< 时钟节拍计数 */

lztList lztTaskDelayedList;                         /**< 延时队列 */

uint32_t idleCount;                             /**< 空闲任务计数 */
uint32_t idleMaxCount;                          /**< 空闲任务最大计数 */

#if LZTOS_ENABLE_CPUUSAGE_STAT == 1
static void initCpuUsageStat (void);
static void checkCpuUsage (void);
static void cpuUsageSyncWithSysTick (void);
#endif

/**
 * @brief 获取当前最高优先级且可运行的任务
 * @return 优先级最高的且可运行的任务
 */
lztTask *lztTaskHighestReady (void) {
    uint32_t highestPrio = lztBitmapGetFirstSet(&taskPrioBitmap);
    lztNode *node = lztListFirst(&taskTable[highestPrio]);
    return (lztTask *) lztNodeParent(node, lztTask, linkNode);
}

/**
 * @brief 初始化调度器
 */
void lztTaskSchedInit (void) {
		int i;
    schedLockCount = 0;
    lztBitmapInit(&taskPrioBitmap);
    for (i = 0; i < LZTOS_PRO_COUNT; i++) {
        lztListInit(&taskTable[i]);
    }
}

/**
 * @brief 禁止任务调度
 */
void lztTaskSchedDisable (void) {
    uint32_t status = lztTaskEnterCritical();

    if (schedLockCount < 255) {
        schedLockCount++;
    }

    lztTaskExitCritical(status);
}

/**
 * @brief 允许任务调度
 */
void lztTaskSchedEnable (void) {
    uint32_t status = lztTaskEnterCritical();

    if (schedLockCount > 0) {
        if (--schedLockCount == 0) {
            lztTaskSched();
        }
    }

    lztTaskExitCritical(status);
}

/**
 * @brief 将任务设置为就绪状态
 * @param task 等待设置为就绪状态的任务
 */
void lztTaskSchedRdy (lztTask *task) {
    lztListAddLast(&taskTable[task->prio], &(task->linkNode));
    lztBitmapSet(&taskPrioBitmap, task->prio);
}

/**
 * @brief 将任务从就绪列表中移除
 * @param task 等待取消就绪的任务
 */
void lztTaskSchedUnRdy (lztTask *task) {
    lztListRemove(&taskTable[task->prio], &(task->linkNode));

    // 队列中可能存在多个任务。只有当没有任务时，才清除位图标记
    if (lztListCount(&taskTable[task->prio]) == 0) {
        lztBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/**
 * @brief 将任务从就绪列表中移除
 * @param task    等待移除的任务
 */
void lztTaskSchedRemove (lztTask *task) {
    lztListRemove(&taskTable[task->prio], &(task->linkNode));

    if (lztListCount(&taskTable[task->prio]) == 0) {
        lztBitmapClear(&taskPrioBitmap, task->prio);
    }
}

/**
 * @brief 任务调度接口。通过它来选择下一个具体的任务，然后切换至该任务运行。
 */
void lztTaskSched (void) {
    lztTask *tempTask;

    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = lztTaskEnterCritical();

    // 如何调度器已经被上锁，则不进行调度，直接退回去
    if (schedLockCount > 0) {
        lztTaskExitCritical(status);
        return;
    }

    // 找到优先级最高的任务。这个任务的优先级可能比当前低低
    // 但是当前任务是因为延时才需要切换，所以必须切换过去，也就是说不能再通过判断优先级来决定是否切换
    // 只要判断不是当前任务，就立即切换过去
    tempTask = lztTaskHighestReady();
    if (tempTask != currentTask) {
        nextTask = tempTask;

#if LZTOS_ENABLE_HOOKS == 1
        lztHooksTaskSwitch(currentTask, nextTask);
#endif
        lztTaskSwitch();
    }

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * @brief 初始化任务延时机制
 */
void lztTaskDelayedInit (void) {
    lztListInit(&lztTaskDelayedList);
}

/**
 * @brief 将任务加入延时队列中
 * @param task    需要延时的任务
 * @param ticks   延时的ticks
 */
void lztTimeTaskWait (lztTask *task, uint32_t ticks) {
    task->delayTicks = ticks;
    lztListAddLast(&lztTaskDelayedList, &(task->delayNode));
    task->state |= LZTOS_TASK_STATE_DELAYED;
}

/**
 * @brief 将延时的任务从延时队列中唤醒
 * @param task  需要唤醒的任务
 */
void lztTimeTaskWakeUp (lztTask *task) {
    task->delayTicks = 0;

    lztListRemove(&lztTaskDelayedList, &(task->delayNode));
    task->state &= ~LZTOS_TASK_STATE_DELAYED;
}

/**
 * @brief 将延时的任务从延时队列中移除
 * @param task  需要移除的任务
 */
void lztTimeTaskRemove (lztTask *task) {
    lztListRemove(&lztTaskDelayedList, &(task->delayNode));
}

/**
 * @brief 初始化时钟节拍计数
 */
void lztTimeTickInit (void) {
    tickCount = 0;
}

/**
 * @brief 系统时钟节拍处理
 */
void lztTaskSystemTickHandler (void) {
    lztNode *node;
    uint32_t count;

    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = lztTaskEnterCritical();

    // 检查所有任务的delayTicks数，如果不0的话，减1。
    for (node = lztListFirst(&lztTaskDelayedList), count = lztListCount(&lztTaskDelayedList); count > 0; count--) {
        lztTask *task = lztNodeParent(node, lztTask, delayNode);
        if (--task->delayTicks == 0) {
            // 如果任务还处于等待事件的状态，则将其从事件等待队列中唤醒
            if (task->waitEvent) {
                // 此时，消息为空，等待结果为超时
                lztEventRemoveTask(task, (void *) 0, lztErrorTimeout);
            }

            // 预先获取下一结点，否则下面的移除会造成问题
            node = lztListNext(&lztTaskDelayedList, node);

            // 将任务从延时队列中移除
            lztTimeTaskWakeUp(task);

            // 将任务恢复到就绪状态
            lztTaskSchedRdy(task);
        }
    }

    // 检查下当前任务的时间片是否已经到了
    if (--currentTask->slice == 0) {
        // 如果当前任务中还有其它任务的话，那么切换到下一个任务
        // 方法是将当前任务从队列的头部移除，插入到尾部
        // 这样后面执行tTaskSched()时就会从头部取出新的任务取出新的任务作为当前任务运行
        if (lztListCount(&taskTable[currentTask->prio]) > 1) {
            lztListRemoveFirst(&taskTable[currentTask->prio]);
            lztListAddLast(&taskTable[currentTask->prio], &(currentTask->linkNode));
        }
        currentTask->slice = LZTOS_SLICE_MAX;
    }

    // 节拍计数增加
    tickCount++;

#if LZTOS_ENABLE_CPUUSAGE_STAT == 1
    // 检查cpu使用率
    checkCpuUsage();
#endif

    // 退出临界区
    lztTaskExitCritical(status);

#if LZTOS_ENABLE_TIMER == 1
    // 通知定时器模块节拍事件
    lztTimerModuleTickNotify();
#endif

#if LZTOS_ENABLE_HOOKS == 1
    lztHooksSysTick();
#endif

    // 这个过程中可能有任务延时完毕(delayTicks = 0)，进行一次调度。
    lztTaskSched();
}

#if LZTOS_ENABLE_CPUUSAGE_STAT == 1

static float cpuUsage;                      /**< cpu使用率统计 */
static uint32_t enableCpuUsageStat;         /**< 是否使能cpu统计 */

/**
 * @brief 初始化cpu统计
 */
static void initCpuUsageStat (void) {
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0;
    enableCpuUsageStat = 0;
}

/**
 * @brief 检查cpu使用率
 */
static void checkCpuUsage (void) {
    // 与空闲任务的cpu统计同步
    if (enableCpuUsageStat == 0) {
        enableCpuUsageStat = 1;
        tickCount = 0;
        return;
    }

    if (tickCount == TICKS_PER_SEC) {
        // 统计最初1s内的最大计数值
        idleMaxCount = idleCount;
        idleCount = 0;

        // 计数完毕，开启调度器，允许切换到其它任务
        lztTaskSchedEnable();
    } else if (tickCount % TICKS_PER_SEC == 0) {
        // 之后每隔1s统计一次，同时计算cpu利用率
        cpuUsage = 100 - (idleCount * 100.0 / idleMaxCount);
        idleCount = 0;
    }
}

/**
 * @brief 为检查cpu使用率与系统时钟节拍同步
 */
static void cpuUsageSyncWithSysTick (void) {
    while (enableCpuUsageStat == 0) {
    }
}

/**
 * @brief 获取cpu使用率,已经计算为百分比
 */
float tCpuUsageGet (void) {
    float usage = 0;

    uint32_t status = tTaskEnterCritical();
    usage = cpuUsage;
    tTaskExitCritical(status);

    return usage;
}

#endif

lztTask lztTaskIdle;                                            /**< 空闲任务结构 */
lztTaskStack idleTaskEnv[LZTOS_IDLETASK_STACK_SIZE];         /**< 空闲任务堆栈 */

/**
 * @brief 空闲任务
 * @param param 空闲任务的初始参数
 */
void idleTaskEntry (void *param) {
    // 禁止调度，防止后面在创建任务时切换到其它任务中去
    lztTaskSchedDisable();

    // 初始化App相关配置
    lztInitApp();

#if LZTOS_ENABLE_TIMER == 1
    // 初始化定时器任务
    lztTimerInitTask();
#endif

    // 启动系统时钟节拍
    lztSetSysTickPeriod(LZTOS_SYSTICK_MS);

#if LZTOS_ENABLE_CPUUSAGE_STAT == 1
    // 等待与时钟同步
    cpuUsageSyncWithSysTick();
#else
    // 开启调度器，允许切换到其它任务
    lztTaskSchedEnable();
#endif

    for (;;) {
        uint32_t status = lztTaskEnterCritical();
        idleCount++;
        lztTaskExitCritical(status);

#if LZTOS_ENABLE_HOOKS == 1
        lztHooksCpuIdle();
#endif
    }
}

/**
 * 获取空闲任务结构
 * @return 空闲任务结构
 */
lztTask * tIdleTask (void) {
    return &lztTaskIdle;
}


/**
 * @brief 系统入口，完成所有功能的创建，空闲任务的创建等功能
 * @return 0, 无用
 */
int main () {
    // 优先初始化lztOS的核心功能
    lztTaskSchedInit();

    // 初始化延时队列
    lztTaskDelayedInit();

#if LZTOS_ENABLE_TIMER == 1
    // 初始化定时器模块
    lztTimerModuleInit();
#endif

    // 初始化时钟节拍
    lztTimeTickInit();

#if LZTOS_ENABLE_CPUUSAGE_STAT == 1
    // 初始化cpu统计
    initCpuUsageStat();
#endif

    // 创建空闲任务
    lztTaskInit(&lztTaskIdle, idleTaskEntry, (void *) 0, LZTOS_PRO_COUNT - 1, idleTaskEnv, sizeof(idleTaskEnv));

    // 这里，不再指定先运行哪个任务，而是自动查找最高优先级的任务运行
    nextTask = lztTaskHighestReady();
    currentTask = (lztTask *) 0;

    // 切换到nextTask， 这个函数永远不会返回
    lztTaskRunFirst();
    return 0;
}

/** @} */
