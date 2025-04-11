/**事件控制块
 */
#include "lztOS.h"

/**
 * 初始化事件控制块
 * @param vent 事件控制块
 * @param type 事件控制块的类型
 */
void lztEventInit (lztEvent *event, lztEventType type) {
    event->type = type;
    lztListInit(&event->waitList);
}

/**
 * 让指定在事件控制块上等待事件发生
 * @param event 事件控制块
 * @param task 等待事件发生的任务
 * @param msg 事件消息存储的具体位置
 * @param state 消息类型
 * @param timeout 等待多长时间
 * @return 优先级最高的且可运行的任务
 */
void lztEventWait (lztEvent *event, lztTask *task, void *msg, uint32_t state, uint32_t timeout) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    task->state |= state;                   // 标记任务处于等待某种事件的状态
    task->waitEvent = event;                // 设置任务等待的事件结构
    task->eventMsg = msg;                   // 设置任务等待事件的消息存储位置
    // 因有时候需要接受消息，所以需要接受区
    task->waitEventResult = lztErrorNoError;  // 清空事件的等待结果

    // 将任务从就绪队列中移除
    lztTaskSchedUnRdy(task);

    // 将任务插入到等待队列中
    lztListAddLast(&event->waitList, &task->linkNode);

    // 如果发现有设置超时，在同时插入到延时队列中
    // 当时间到达时，由延时处理机制负责将任务从延时列表中移除，同时从事件列表中移除
    if (timeout) {
        lztTimeTaskWait(task, timeout);
    }

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 从事件控制块中唤醒首个等待的任务
 * @param event 事件控制块
 * @param msg 事件消息
 * @param result 告知事件的等待结果
 * @return 首个等待的任务，如果没有任务等待，则返回0
 */
lztTask *lztEventWakeUp (lztEvent *event, void *msg, uint32_t result) {
    lztNode *node;
    lztTask *task = (lztTask *) 0;

    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 取出等待队列中的第一个结点
    if ((node = lztListRemoveFirst(&event->waitList)) != (lztNode *) 0) {
        // 转换为相应的任务结构
        task = (lztTask *) lztNodeParent(node, lztTask, linkNode);

        // 设置收到的消息、结构，清除相应的等待标志位
        task->waitEvent = (lztEvent *) 0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~LZTOS_TASK_WAIT_MASK;

        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->delayTicks != 0) {
            lztTimeTaskWakeUp(task);
        }

        // 将任务加入就绪队列
        lztTaskSchedRdy(task);
    }

    // 退出临界区
    lztTaskExitCritical(status);

    return task;
}

/**
 * 从事件控制块中唤醒指定任务
 * @param event 事件控制块
 * @param task 等待唤醒的任务
 * @param msg 事件消息
 * @param result 告知事件的等待结果
 * @return 首个等待的任务，如果没有任务等待，则返回0
 */
void lztEventWakeUpTask (lztEvent *event, lztTask *task, void *msg, uint32_t result) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    lztListRemove(&event->waitList, &task->linkNode);

    // 设置收到的消息、结构，清除相应的等待标志位
    task->waitEvent = (lztEvent *) 0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~LZTOS_TASK_WAIT_MASK;

    // 任务申请了超时等待，这里检查下，将其从延时队列中移除
    if (task->delayTicks != 0) {
        lztTimeTaskWakeUp(task);
    }

    // 将任务加入就绪队列
    lztTaskSchedRdy(task);

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 将任务从其等待队列中强制移除
 * @param task 待移除的任务
 * @param result 告知事件的等待结果
 */
void lztEventRemoveTask (lztTask *task, void *msg, uint32_t result) {
    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 将任务从所在的等待队列中移除
    // 注意，这里没有检查waitEvent是否为空。既然是从事件中移除，那么认为就不可能为空
    lztListRemove(&task->waitEvent->waitList, &task->linkNode);

    // 设置收到的消息、结构，清除相应的等待标志位
    task->waitEvent = (lztEvent *) 0;
    task->eventMsg = msg;
    task->waitEventResult = result;
    task->state &= ~LZTOS_TASK_WAIT_MASK;

    // 退出临界区
    lztTaskExitCritical(status);
}

/**
 * 清除所有等待中的任务，将事件发送给所有任务
 * @param event 事件控制块
 * @param msg 事件消息
 * @param result 告知事件的等待结果
 * @return 唤醒的任务数量
 */
uint32_t lztEventRemoveAll (lztEvent *event, void *msg, uint32_t result) {
    lztNode *node;
    uint32_t count;

    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    // 获取等待中的任务数量
    count = lztListCount(&event->waitList);

    // 遍历所有等待中的任务
    while ((node = lztListRemoveFirst(&event->waitList)) != (lztNode *) 0) {
        // 转换为相应的任务结构                                          
        lztTask *task = (lztTask *) lztNodeParent(node, lztTask, linkNode);

        // 设置收到的消息、结构，清除相应的等待标志位
        task->waitEvent = (lztEvent *) 0;
        task->eventMsg = msg;
        task->waitEventResult = result;
        task->state &= ~LZTOS_TASK_WAIT_MASK;

        // 任务申请了超时等待，这里检查下，将其从延时队列中移除
        if (task->delayTicks != 0) {
            lztTimeTaskWakeUp(task);
        }

        // 将任务加入就绪队列
        lztTaskSchedRdy(task);
    }

    // 退出临界区
    lztTaskExitCritical(status);

    return count;
}

/**
 * 事件控制块中等待的任务数量
 * @param event 事件控制块
 * @param msg 事件消息
 * @param result 告知事件的等待结果
 * @return 唤醒的任务数量
 */
uint32_t lztEventWaitCount (lztEvent *event) {
    uint32_t count = 0;

    // 进入临界区
    uint32_t status = lztTaskEnterCritical();

    count = lztListCount(&event->waitList);

    // 退出临界区
    lztTaskExitCritical(status);

    return count;
}


