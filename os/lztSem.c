//信号量 计数信号量

#include "lztSem.h"
#include "lztOS.h"

#if !defined(LZTOS_ENABLE_SEM) || LZTOS_ENABLE_SEM == 1

/**
 * 初始化信号量
 * @param startCount 初始的计数
 * @param maxCount 最大计数，如果为0，则不限数量
 */
void lztSemInit (lztSem *sem, uint32_t startCount, uint32_t maxCount) {
    lztEventInit(&sem->event, lztEventTypeSem);

    sem->maxCount = maxCount;
    if (maxCount == 0) {
        sem->count = startCount;
    } else {
        sem->count = (startCount > maxCount) ? maxCount : startCount;
    }
}

/**
 * 等待信号量
 * @param sem 等待的信号量
 * @param waitTicks 当信号量计数为0时，等待的ticks数，为0时表示永远等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t lztSemWait (lztSem *sem, uint32_t waitTicks) {
    uint32_t status = lztTaskEnterCritical();

    // 首先检查信号量计数是否大于0
    if (sem->count > 0) {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        lztTaskExitCritical(status);
        return lztErrorNoError;
    } else {
        // 然后将任务插入事件队列中
        lztEventWait(&sem->event, currentTask, (void *) 0, lztEventTypeSem, waitTicks);
        lztTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        lztTaskSched();

        // 当由于等待超时或者计数可用时，执行会返回到这里，然后取出等待结构
        return currentTask->waitEventResult;
    }
}

/**
 * 获取信号量，如果信号量计数不可用，则立即退回
 * @param sem 等待的信号量
 * @return 获取结果, tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t lztSemNoWaitGet (lztSem *sem) {
    uint32_t status = lztTaskEnterCritical();

    // 首先检查信号量计数是否大于0
    if (sem->count > 0) {
        // 如果大于0的话，消耗掉一个，然后正常退出
        --sem->count;
        lztTaskExitCritical(status);
        return lztErrorNoError;
    } else {
        // 否则，返回资源不可用
        lztTaskExitCritical(status);
        return lztErrorResourceUnavaliable;
    }
}

/**
 * 通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
 * @param sem 操作的信号量
 */
void lztSemNotify (lztSem *sem) {
    uint32_t status = lztTaskEnterCritical();

    // 检查是否有任务等待
    if (lztEventWaitCount(&sem->event) > 0) {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        lztTask *task = lztEventWakeUp(&sem->event, (void *) 0, lztErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio) {
            lztTaskSched();
        }
    } else {
        // 如果没有任务等待的话，增加计数
        ++sem->count;

        // 如果这个计数超过了最大允许的计数，则递减
        if ((sem->maxCount != 0) && (sem->count > sem->maxCount)) {
            sem->count = sem->maxCount;
        }
    }

    lztTaskExitCritical(status);
}

/**
 * 查询信号量的状态信息
 * @param sem 查询的信号量
 * @param info 状态查询存储的位置
 */
void lztSemGetInfo (lztSem *sem, lztSemInfo *info) {
    uint32_t status = lztTaskEnterCritical();

    // 拷贝需要的信息
    info->count = sem->count;
    info->maxCount = sem->maxCount;
    info->taskCount = lztEventWaitCount(&sem->event);

    lztTaskExitCritical(status);
}

/**
 * 销毁信号量
 * @param sem 需要销毁的信号量
 * @return 因销毁该信号量而唤醒的任务数量
 */
uint32_t lztSemDestroy (lztSem *sem) {
    uint32_t status = lztTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = lztEventRemoveAll(&sem->event, (void *) 0, lztErrorDel);
    sem->count = 0;
    lztTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) {
        lztTaskSched();
    }
    return count;
}

#endif

/** @} */
