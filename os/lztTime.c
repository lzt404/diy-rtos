//任务管理

#include "lztOS.h"

/**
 * 使当前任务进入延时状态
 * @param delay 延时多少个ticks
 */
void lztTaskDelay (uint32_t delay) {
    // 进入临界区，以保护在整个任务调度与切换期间，不会因为发生中断导致currentTask和nextTask可能更改
    uint32_t status = lztTaskEnterCritical();

    // 设置延时值，插入延时队列
    lztTimeTaskWait(currentTask, delay);

    // 将任务从就绪表中移除
    lztTaskSchedUnRdy(currentTask);

    // 然后进行任务切换，切换至另一个任务，或者空闲任务
    // delayTikcs会在时钟中断中自动减1.当减至0时，会切换回来继续运行。
    lztTaskSched();

    // 退出临界区
    lztTaskExitCritical(status);
}

