//任务管理

#ifndef LZTTASK_H
#define LZTTASK_H
#include <stdint.h>
#define LZTOS_TASK_STATE_RDY                   0
#define LZTOS_TASK_STATE_DESTROYED             (1 << 0)
#define LZTOS_TASK_STATE_DELAYED               (1 << 1)
#define LZTOS_TASK_STATE_SUSPEND               (1 << 2)

#define LZTOS_TASK_WAIT_MASK                   (0xFF << 16)

// 前置声明
struct _lztEvent;

// Cortex-M的堆栈单元类型：堆栈单元的大小为32位，所以使用uint32_t
typedef uint32_t lztTaskStack;

// 任务结构：包含了一个任务的所有信息
typedef struct _lztTask {
    // 任务所用堆栈的当前堆栈指针。每个任务都有他自己的堆栈，用于在运行过程中存储临时变量等一些环境参数
    // 在lztOS运行该任务前，会从stack指向的位置处，会读取堆栈中的环境参数恢复到CPU寄存器中，然后开始运行
    // 在切换至其它任务时，会将当前CPU寄存器值保存到堆栈中，等待下一次运行该任务时再恢复。
    // stack保存了最后保存环境参数的地址位置，用于后续恢复
    uint32_t *stack;
    uint32_t *stackBase;                /**< 堆栈的起即地址 */

    uint32_t stackSize;                 /**< 堆栈的总容量 */
    lztNode linkNode;                     /**< 连接结点 */

    uint32_t delayTicks;                /**< 任务延时计数器 */
    lztNode delayNode;                    /**< 延时结点：通过delayNode就可以将tTask放置到延时队列中 */

    uint32_t prio;                      /**< 任务的优先级 */

    uint32_t state;                     /**< 任务当前状态 */
    uint32_t slice;                     /**< 当前剩余的时间片 */

    uint32_t suspendCount;              /**< 被挂起的次数 */

    void (*clean) (void *param);        /**< 任务被删除时调用的清理函数 */
    void *cleanParam;                   /**< 传递给清理函数的参数 */
    uint8_t requestDeleteFlag;          /**< 请求删除标志，非0表示请求删除 */


    struct _lztEvent *waitEvent;          /**< 任务正在等待的事件类型 */
    void *eventMsg;                     /**< 等待事件的消息存储位置 */
    uint32_t waitEventResult;           /**< 等待事件的结 */
    uint32_t waitFlagsType;             /**< 等待的事件方式 */
    uint32_t eventFlags;                /**< 等待的事件标志 */
} lztTask;

// 任务相关信息结构
typedef struct _lztTaskInfo {
    uint32_t delayTicks;                /**< 任务延时计数器 */
    uint32_t prio;                      /**< 任务的优先级 */
    uint32_t state;                     /**< 任务当前状态 */
    uint32_t slice;                     /**< 当前剩余的时间片 */
    uint32_t suspendCount;              /**< 被挂起的次数 */
    uint32_t stackSize;                 /**< 堆栈的总容量 */
    uint32_t stackFree;                 /**< 堆栈空余量 */
} lztTaskInfo;

void lztTaskInit (lztTask *task, void (*entry) (void *), void *param, uint32_t prio, uint32_t *stack, uint32_t size);
void lztTaskSuspend (lztTask *task);
void lztTaskWakeUp (lztTask *task);
void lztTaskSetCleanCallFunc (lztTask *task, void (*clean) (void *param), void *param);
void lztTaskForceDelete (lztTask *task);
void lztTaskRequestDelete (lztTask *task);
uint8_t lztTaskIsRequestedDelete (void);
void lztTaskDeleteSelf (void);
void lztTaskGetInfo (lztTask *task, lztTaskInfo *info);

#endif
