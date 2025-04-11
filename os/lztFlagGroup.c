//事件标志组 事件标志组

#include "lztFlagGroup.h"
#include "lztOS.h"

#if !defined(LZTOS_ENABLE_FLAGGROUP) || LZTOS_ENABLE_FLAGGROUP == 1

/**
 * 初始化事件标志组
 * @param flagGroup 等待初始化的事件标志组
 * @param flags 初始事件标志
 */
void lztFlagGroupInit (lztFlagGroup *flagGroup, uint32_t flags) {
    lztEventInit(&flagGroup->event, lztEventTypeFlagGroup);
    flagGroup->flags = flags;
}

/**
 * 等待事件标志组中特定的标志
 * @param flagGroup 等待的事件标志组
 * @param waitType 等待的事件类型
 * @param requstFlag 请求的事件标志
 * @param resultFlag 等待标志结果
 * @param waitTicks 当等待的标志没有满足条件时，等待的ticks数，为0时表示永远等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
static uint32_t lztFlagGroupCheckAndConsume (lztFlagGroup *flagGroup, uint32_t type, uint32_t *flags) {
    uint32_t srcFlags = *flags;
    uint32_t isSet = type & LZTFLAGGROUP_SET;
    uint32_t isAll = type & LZTFLAGGROUP_ALL;
    uint32_t isConsume = type & LZTFLAGGROUP_CONSUME;

    // 有哪些类型的标志位出现
    // flagGroup->flags & flags：计算出哪些位为1
    // ~flagGroup->flags & flags:计算出哪位为0
    uint32_t calcFlag = isSet ? (flagGroup->flags & srcFlags) : (~flagGroup->flags & srcFlags);

    // 所有标志位出现, 或者做任意标志位出现，满足条件
    if (((isAll != 0) && (calcFlag == srcFlags)) || ((isAll == 0) && (calcFlag != 0))) {
        // 是否消耗掉标志位
        if (isConsume) {
            if (isSet) {
                // 清除为1的标志位，变成0
                flagGroup->flags &= ~srcFlags;
            } else {
                // 清除为0的标志位，变成1
                flagGroup->flags |= srcFlags;
            }
        }
        *flags = calcFlag;
        return lztErrorNoError;
    }

    *flags = calcFlag;
    return lztErrorResourceUnavaliable;
}

/**
 * 获取事件标志组中特定的标志
 * @param flagGroup 获取的事件标志组
 * @param waitType 获取的事件类型
 * @param requstFlag 请求的事件标志
 * @param resultFlag 等待标志结果
 * @param watiTicks 当条件不满足时，最大等待多少个ticks时间
 * @param 获取结果,tErrorResourceUnavaliable.tErrorNoError
 */
uint32_t lztFlagGroupWait (lztFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag,
                         uint32_t *resultFlag, uint32_t waitTicks) {
    uint32_t result;
    uint32_t flags = requestFlag;

    uint32_t status = lztTaskEnterCritical();
    result = lztFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    if (result != lztErrorNoError) {
        // 如果事件标志不满足条件，则插入到等待队列中
        currentTask->waitFlagsType = waitType;
        currentTask->eventFlags = requestFlag;
        lztEventWait(&flagGroup->event, currentTask, (void *) 0, lztEventTypeFlagGroup, waitTicks);

        lztTaskExitCritical(status);

        // 再执行一次事件调度，以便于切换到其它任务
        lztTaskSched();

        *resultFlag = currentTask->eventFlags;
        result = currentTask->waitEventResult;
    } else {
        *resultFlag = flags;
        lztTaskExitCritical(status);
    }

    return result;
}

/**
 * 无阻塞获取事件标志组中特定的标志，当条件不满足时，立即退出
 * @param flagGroup 获取的事件标志组
 * @param waitType 获取的事件类型
 * @param requstFlag 请求的事件标志
 * @param resultFlag 等待标志结果
 * @param 获取结果,tErrorResourceUnavaliable.tErrorNoError
 */
uint32_t lztFlagGroupNoWaitGet (lztFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag, uint32_t *resultFlag) {
    uint32_t flags = requestFlag;

    uint32_t status = lztTaskEnterCritical();
    uint32_t result = lztFlagGroupCheckAndConsume(flagGroup, waitType, &flags);
    lztTaskExitCritical(status);

    *resultFlag = flags;
    return status;
}

/**
 * 通知事件标志组相应的事件标志设置或者清0。如果等待队列中有任务，则对任务进行处理
 * @param flagGroup 事件标志组
 * @param isSet 事件标志是否置位
 * @param flags 哪些事件标志位发生了变化
 */
void lztFlagGroupNotify (lztFlagGroup *flagGroup, uint8_t isSet, uint32_t flags) {
    lztList *waitList;
    lztNode *node;
    lztNode *nextNode;
    uint8_t sched = 0;

    uint32_t status = lztTaskEnterCritical();

    if (isSet) {
        flagGroup->flags |= flags;     // 置1事件
    } else {
        flagGroup->flags &= ~flags;    // 清0事件
    }

    // 遍历所有的等待任务, 获取满足条件的任务，加入到待移除列表中
    waitList = &flagGroup->event.waitList;
    for (node = waitList->headNode.nextNode; node != &(waitList->headNode); node = nextNode) {
        uint32_t result;
        lztTask *task = lztNodeParent(node, lztTask, linkNode);
        uint32_t flags = task->eventFlags;
        nextNode = node->nextNode;

        // 检查标志
        result = lztFlagGroupCheckAndConsume(flagGroup, task->waitFlagsType, &flags);
        if (result == lztErrorNoError) {
            // 唤醒任务
            task->eventFlags = flags;
            lztEventWakeUpTask(&flagGroup->event, task, (void *) 0, lztErrorNoError);
            sched = 1;
        }
    }

    // 如果有任务就绪，则执行一次调度
    if (sched) {
        lztTaskSched();
    }

    lztTaskExitCritical(status);
}

/**
 * 查询事件标志组的状态信息
 * @param flagGroup 事件标志组
 * @param info 状态查询存储的位置
 */
void lztFlagGroupGetInfo (lztFlagGroup *flagGroup, lztFlagGroupInfo *info) {
    uint32_t status = lztTaskEnterCritical();

    // 拷贝需要的信息
    info->flags = flagGroup->flags;
    info->taskCount = lztEventWaitCount(&flagGroup->event);

    lztTaskExitCritical(status);
}

/**
 * 销毁事件标志组
 * @param flagGroup 事件标志组
 * @return 因销毁该存储控制块而唤醒的任务数量
 */
uint32_t lztFlagGroupDestroy (lztFlagGroup *flagGroup) {
    uint32_t status = lztTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = lztEventRemoveAll(&flagGroup->event, (void *) 0, lztErrorDel);

    lztTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) {
        lztTaskSched();
    }
    return count;
}

#endif

