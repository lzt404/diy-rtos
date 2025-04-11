//任务管理


#include "lztOS.h"
#include <string.h>
/**
 * 初始化任务结构
 * @param task 要初始化的任务结构
 * @param entry 任务的入口函数
 * @param param 传递给任务的运行参数
 */
void lztTaskInit (lztTask *task, void (*entry) (void *), void *param, uint32_t prio, uint32_t *stack, uint32_t size) {
    uint32_t *stackTop;

    // 为了简化代码，lztOS无论是在启动时切换至第一个任务，还是在运行过程中在不同间任务切换
    // 所执行的操作都是先保存当前任务的运行环境参数（CPU寄存器值）的堆栈中(如果已经运行运行起来的话)，然后再
    // 取出从下一个任务的堆栈中取出之前的运行环境参数，然后恢复到CPU寄存器
    // 对于切换至之前从没有运行过的任务，我们为它配置一个“虚假的”保存现场，然后使用该现场恢复。
    task->stackBase = stack;
    task->stackSize = size;
    memset(stack, 0, size);

    // 注意以下两点：
    // 1、不需要用到的寄存器，直接填了寄存器号，方便在IDE调试时查看效果；
    // 2、顺序不能变，结合PendSV_Handler以及CPU对异常的处理流程来理解
    stackTop = stack + size / sizeof(lztTaskStack);
    *(--stackTop) = (unsigned long) (1 << 24);               // XPSR, 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行
    *(--stackTop) = (unsigned long) entry;                  // 程序的入口地址
    *(--stackTop) = (unsigned long) 0x14;                   // R14(LR), 任务不会通过return xxx结束自己，所以未用
    *(--stackTop) = (unsigned long) 0x12;                   // R12, 未用
    *(--stackTop) = (unsigned long) 0x3;                    // R3, 未用
    *(--stackTop) = (unsigned long) 0x2;                    // R2, 未用
    *(--stackTop) = (unsigned long) 0x1;                    // R1, 未用
    *(--stackTop) = (unsigned long) param;                  // R0 = param, 传给任务的入口函数
    *(--stackTop) = (unsigned long) 0x11;                   // R11, 未用
    *(--stackTop) = (unsigned long) 0x10;                   // R10, 未用
    *(--stackTop) = (unsigned long) 0x9;                    // R9, 未用
    *(--stackTop) = (unsigned long) 0x8;                    // R8, 未用
    *(--stackTop) = (unsigned long) 0x7;                    // R7, 未用
    *(--stackTop) = (unsigned long) 0x6;                    // R6, 未用
    *(--stackTop) = (unsigned long) 0x5;                    // R5, 未用
    *(--stackTop) = (unsigned long) 0x4;                    // R4, 未用

    task->slice = LZTOS_SLICE_MAX;                     // 初始化任务的时间片计数
    task->stack = stackTop;                             // 保存最终的值
    task->prio = prio;                                  // 设置任务的优先级
    task->state = LZTOS_TASK_STATE_RDY;                // 设置任务为就绪状态
    task->suspendCount = 0;                             // 初始挂起次数为0
    task->clean = (void (*) (void *)) 0;                   // 设置清理函数
    task->cleanParam = (void *) 0;                       // 设置传递给清理函数的参数
    task->requestDeleteFlag = 0;                        // 请求删除标记

    task->waitEvent = (lztEvent *) 0;                      // 没有等待事件
    task->eventMsg = (void *) 0;                         // 没有等待事件
    task->waitEventResult = lztErrorNoError;              // 没有等待事件错误

    lztNodeInit(&(task->delayNode));                      // 初始化延时结点

    lztNodeInit(&(task->linkNode));                       // 初始化链接结点

    lztTaskSchedRdy(task);                                // 将任务插入就绪队列

#if LZTOS_ENABLE_HOOKS == 1
    lztHooksTaskInit(task);
#endif
}

/**
 * 挂起指定的任务
 * @param task 待挂起的任务
 */
void lztTaskSuspend (lztTask *task) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 不允许对已经进入延时状态的任务挂起
    if (!(task->state & LZTOS_TASK_STATE_DELAYED)) {
        // 增加挂起计数，仅当该任务被执行第一次挂起操作时，才考虑是否
        // 要执行任务切换操作
        if (++task->suspendCount <= 1) {
            // 设置挂起标志
            task->state |= LZTOS_TASK_STATE_SUSPEND;

            // 挂起方式很简单，就是将其从就绪队列中移除，这样调度器就不会发现他
            // 也就没法切换到该任务运行
            lztTaskSchedUnRdy(task);

            // 当然，这个任务可能是自己，那么就切换到其它任务
            if (task == currentTask) {
                lztTaskSched();
            }
        }
    }

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 唤醒被挂起的任务
 * @param task 待唤醒的任务
 */
void lztTaskWakeUp (lztTask *task) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 检查任务是否处于挂起状态
    if (task->state & LZTOS_TASK_STATE_SUSPEND) {
        // 递减挂起计数，如果为0了，则清除挂起标志，同时设置进入就绪状态
        if (--task->suspendCount == 0) {
            // 清除挂起标志
            task->state &= ~LZTOS_TASK_STATE_SUSPEND;

            // 同时将任务放回就绪队列中
            lztTaskSchedRdy(task);

            // 唤醒过程中，可能有更高优先级的任务就绪，执行一次任务调度
            lztTaskSched();
        }
    }

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 设置任务被删除时调用的清理函数
 * @param task  待设置的任务
 * @param clean  清理函数入口地址
 * @param param  传递给清理函数的参数
 */
void lztTaskSetCleanCallFunc (lztTask *task, void (*clean) (void *param), void *param) {
    task->clean = clean;
    task->cleanParam = param;
}

/**
 * 强制删除指定的任务
 * @param task  task  需要删除的任务
 */
void lztTaskForceDelete (lztTask *task) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 如果任务处于延时状态，则从延时队列中删除
    if (task->state & LZTOS_TASK_STATE_DELAYED) {
        lztTimeTaskRemove(task);
    }
        // 如果任务不处于挂起状态，那么就是就绪态，从就绪表中删除
    else if (!(task->state & LZTOS_TASK_STATE_SUSPEND)) {
        lztTaskSchedRemove(task);
    }


    // 删除时，如果有设置清理函数，则调用清理函数
    if (task->clean) {
        task->clean(task->cleanParam);
    }

    // 如果删除的是自己，那么需要切换至另一个任务，所以执行一次任务调度
    if (currentTask == task) {
        lztTaskSched();
    }

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 请求删除某个任务，由任务自己决定是否删除自己
 */
void lztTaskRequestDelete (lztTask *task) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 设置清除删除标记
    task->requestDeleteFlag = 1;

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 是否已经被请求删除自己
 * @param ask  需要删除的任务
 * @return 非0表示请求删除，0表示无请求
 */
uint8_t lztTaskIsRequestedDelete (void) {
    uint8_t delete;

    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 获取请求删除标记
    delete = currentTask->requestDeleteFlag;

    // 退出临界区
    lztTaskExitCritical(status);

    return delete;
}

/**
 * 删除自己
 */
void lztTaskDeleteSelf (void) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 任务在调用该函数时，必须是处于就绪状态，不可能处于延时或挂起等其它状态
    // 所以，只需要从就绪队列中移除即可
    lztTaskSchedRemove(currentTask);

    // 删除时，如果有设置清理函数，则调用清理函数
    if (currentTask->clean) {
        currentTask->clean(currentTask->cleanParam);
    }

    // 接下来，肯定是切换到其它任务去运行
    lztTaskSched();

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 获取任务相关信息
 * @param task 需要查询的任务
 * @param info 任务信息存储结构
 */
void lztTaskGetInfo (lztTask *task, lztTaskInfo *info) {
    uint32_t *stackEnd;

    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    info->delayTicks = task->delayTicks;                // 延时信息
    info->prio = task->prio;                            // 任务优先级
    info->state = task->state;                          // 任务状态
    info->slice = task->slice;                          // 剩余时间片
    info->suspendCount = task->suspendCount;            // 被挂起的次数
    info->stackSize = task->stackSize;

    // 计算堆栈使用量
    info->stackFree = 0;
    stackEnd = task->stackBase;
    while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(lztTaskStack))) {
        info->stackFree++;
    }

    // 转换成字节数
    info->stackFree *= sizeof(lztTaskStack);

    // 退出临界区
    lztTaskExitCritical(status);
}

