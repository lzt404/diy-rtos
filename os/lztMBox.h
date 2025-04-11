//邮箱

#ifndef TMBOX_H
#define TMBOX_H

#include "lztConfig.h"

#if !defined(LZTOS_ENABLE_MBOX) || LZTOS_ENABLE_MBOX == 1

#include "lztEvent.h"

#define tMBOXSendNormal             0x00        /**< 正常发送发送至缓冲区 */
#define tMBOXSendFront              0x01        /**< 消息发送至缓冲区头部 */

// 邮箱类型
typedef struct _lztMbox {
    lztEvent event;                               /**< 事件控制块 */

    uint32_t count;                             /**< 当前的消息数量 */
    uint32_t read;                              /**< 读取消息的索引 */
    uint32_t write;                             /**< 写消息的索引 */
    uint32_t maxCount;                          /**< 最大允许容纳的消息数量 */

    void **msgBuffer;                           /**< 消息存储缓冲区 */
} lztMbox;

// 邮箱状态类型
typedef struct _lztMboxInfo {
    uint32_t count;                             /**< 当前的消息数量 */
    uint32_t maxCount;                          /**< 最大允许容纳的消息数量 */
    uint32_t taskCount;                         /**< 当前等待的任务计数 */
} lztMboxInfo;

void lztMboxInit (lztMbox *mbox, void **msgBuffer, uint32_t maxCount);
uint32_t lztMboxWait (lztMbox *mbox, void **msg, uint32_t waitTicks);
uint32_t lztMboxNoWaitGet (lztMbox *mbox, void **msg);
uint32_t lztMboxNotify (lztMbox *mbox, void *msg, uint32_t notifyOption);
void lztMboxFlush (lztMbox *mbox);
uint32_t lztMboxDestroy (lztMbox *mbox);
void lztMboxGetInfo (lztMbox *mbox, lztMboxInfo *info);

#endif
#endif
