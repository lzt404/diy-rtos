//互斥信号量 互斥信号量

#include "lztMutex.h"
#include "lztOS.h"

#if !defined(LZTOS_ENABLE_MUTEX) || LZTOS_ENABLE_MUTEX == 1

/**
 * 初始化互斥信号量
 * @param mutex 等待初始化的互斥信号量
 */
void lztMutexInit (lztMutex *mutex) {
    lztEventInit(&mutex->event, lztEventTypeMutex);

    mutex->lockedCount = 0;
    mutex->owner = (lztTask *) 0;
    mutex->ownerOriginalPrio = LZTOS_PRO_COUNT;
}

/*优先级继承,当高优先级任务因等待互斥信号量而被阻塞时，
低优先级任务会暂时提升自己的优先级到与高优先级任务相同的水平，以尽快完成资源的使用并释放互斥信号量。
一旦低优先级任务完成了对资源的访问，
它的优先级就会恢复到原来的级别。*/

/**
 * 等待信号量
 * @param mutex 等待的信号量
 * @param waitTicks 最大等待的ticks数，为0表示无限等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t lztMutexWait (lztMutex *mutex, uint32_t waitTicks) {
    uint32_t status = lztTaskEnterCritical();

    if (mutex->lockedCount <= 0) {
        // 如果没有锁定，则使用当前任务锁定
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount++;

        lztTaskExitCritical(status);
        return lztErrorNoError;
    } else {
        // 信号量已经被锁定
        if (mutex->owner == currentTask) {
            // 如果是信号量的拥有者再次wait，简单增加计数
            mutex->lockedCount++;

            lztTaskExitCritical(status);
            return lztErrorNoError;
        } else {
            // 如果是信号量拥有者之外的任务wait，则要检查下是否需要使用
            // 优先级继承方式处理
            if (currentTask->prio < mutex->owner->prio) {
                lztTask *owner = mutex->owner;

                // 如果当前任务的优先级比拥有者优先级更高，则使用优先级继承
                // 提升原拥有者的优先
                if (owner->state == LZTOS_TASK_STATE_RDY) {
                    // 任务处于就绪状态时，更改任务在就绪表中的位置
                    lztTaskSchedUnRdy(owner);
                    owner->prio = currentTask->prio;
                    lztTaskSchedRdy(owner);
                } else {
                    // 其它状态，只需要修改优先级
                    owner->prio = currentTask->prio;
                }
            }

            // 当前任务进入等待队列中
            lztEventWait(&mutex->event, currentTask, (void *) 0, lztEventTypeMutex, waitTicks);
            lztTaskExitCritical(status);

            // 执行调度， 切换至其它任务
            lztTaskSched();
            return currentTask->waitEventResult;
        }
    }
}

/**
 * 获取信号量，如果没有获取到直接退出，不阻塞
 * @param mutex 等待的信号量
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError
 */
uint32_t lztMutexNoWaitGet (lztMutex *mutex) {
    uint32_t status = lztTaskEnterCritical();

    if (mutex->lockedCount <= 0) {
        // 如果没有锁定，则使用当前任务锁定
        mutex->owner = currentTask;
        mutex->ownerOriginalPrio = currentTask->prio;
        mutex->lockedCount++;

        lztTaskExitCritical(status);
        return lztErrorNoError;
    } else {
        // 信号量已经被锁定
        if (mutex->owner == currentTask) {
            // 如果是信号量的拥有者再次wait，简单增加计数
            mutex->lockedCount++;

            lztTaskExitCritical(status);
            return lztErrorNoError;
        }

        lztTaskExitCritical(status);
        return lztErrorResourceUnavaliable;
    }
}

/**
 * 通知互斥信号量可用
 * @param mutex 操作的信号量
 * @return tErrorOwner/tErrorNoError
 */
uint32_t lztMutexNotify (lztMutex *mutex) {
    uint32_t status = lztTaskEnterCritical();

    if (mutex->lockedCount <= 0) {
        // 锁定计数为0，信号量未被锁定，直接退出
        lztTaskExitCritical(status);
        return lztErrorNoError;
    }

    if (mutex->owner != currentTask) {
        // 不是拥有者释放，认为是非法
        lztTaskExitCritical(status);
        return lztErrorOwner;
    }

    if (--mutex->lockedCount > 0) {
        // 减1后计数仍不为0, 直接退出，不需要唤醒等待的任务
        lztTaskExitCritical(status);
        return lztErrorNoError;
    }

    // 是否有发生优先级继承
    if (mutex->ownerOriginalPrio != mutex->owner->prio) {
        // 有发生优先级继承，恢复拥有者的优先级
        if (mutex->owner->state == LZTOS_TASK_STATE_RDY) {
            // 任务处于就绪状态时，更改任务在就绪表中的位置
            lztTaskSchedUnRdy(mutex->owner);
            currentTask->prio = mutex->ownerOriginalPrio;
            lztTaskSchedRdy(mutex->owner);
        } else {
            // 其它状态，只需要修改优先级
            currentTask->prio = mutex->ownerOriginalPrio;
        }
    }

    // 检查是否有任务等待
    if (lztEventWaitCount(&mutex->event) > 0) {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        lztTask *task = lztEventWakeUp(&mutex->event, (void *) 0, lztErrorNoError);

        mutex->owner = task;
        mutex->ownerOriginalPrio = task->prio;
        mutex->lockedCount++;

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio) {
            lztTaskSched();
        }
    }
    lztTaskExitCritical(status);
    return lztErrorNoError;
}

/**
 * 销毁信号量
 * @param mutex 销毁互斥信号量
 * @return 因销毁该信号量而唤醒的任务数量
 */
uint32_t lztMutexDestroy (lztMutex *mutex) {
    uint32_t count = 0;
    uint32_t status = lztTaskEnterCritical();

    // 信号量是否已经被锁定，未锁定时没有任务等待，不必处理
    if (mutex->lockedCount > 0) {
        // 是否有发生优先级继承?如果发生，需要恢复拥有者的原优先级
        if (mutex->ownerOriginalPrio != mutex->owner->prio) {
            // 有发生优先级继承，恢复拥有者的优先级
            if (mutex->owner->state == LZTOS_TASK_STATE_RDY) {
                // 任务处于就绪状态时，更改任务在就绪表中的位置
                lztTaskSchedUnRdy(mutex->owner);
                mutex->owner->prio = mutex->ownerOriginalPrio;
                lztTaskSchedRdy(mutex->owner);
            } else {
                // 其它状态，只需要修改优先级
                mutex->owner->prio = mutex->ownerOriginalPrio;
            }
        }

        // 然后，清空事件控制块中的任务
        count = lztEventRemoveAll(&mutex->event, (void *) 0, lztErrorDel);

        // 清空过程中可能有任务就绪，执行一次调度
        if (count > 0) {
            lztTaskSched();
        }
    }

    lztTaskExitCritical(status);
    return count;
}

/**
 * 查询状态信息
 * @param mutex 查询的互斥信号量
 * @param info 状态查询存储的位置
 */
void lztMutexGetInfo (lztMutex *mutex, lztMutexInfo *info) {
    uint32_t status = lztTaskEnterCritical();

    // 拷贝需要的信息
    info->taskCount = lztEventWaitCount(&mutex->event);
    info->ownerPrio = mutex->ownerOriginalPrio;
    if (mutex->owner != (lztTask *) 0) {
        info->inheritedPrio = mutex->owner->prio;
    } else {
        info->inheritedPrio = LZTOS_PRO_COUNT;
    }
    info->owner = mutex->owner;
    info->lockedCount = mutex->lockedCount;

    lztTaskExitCritical(status);
}

#endif


