//存储块

#include "lztMemBlock.h"
#include "lztOS.h"

#if !defined(LZTOS_ENABLE_MEMBLOCK) || LZTOS_ENABLE_MEMBLOCK == 1

/**
 * 初始化存储控制块
 * @param memBlock 等待初始化的存储控制块
 * @param memStart 存储区的起始地址
 * @param blockSize 每个块的大小
 * @param blockCnt 总的块数量
 * @return 唤醒的任务数量
 */
void lztMemBlockInit (lztMemBlock *memBlock, void *memStart, uint32_t blockSize, uint32_t blockCnt) {
    uint8_t *memBlockStart = (uint8_t *) memStart;
    uint8_t *memBlockEnd = memBlockStart + blockSize * blockCnt;

    // 每个存储块需要来放置链接指针，所以空间至少要比tNode大
    // 即便如此，实际用户可用的空间并没有少
    if (blockSize < sizeof(lztSnode)) {
        return;
    }

    lztEventInit(&memBlock->event, lztEventTypeMemBlock);

    memBlock->memStart = memStart;
    memBlock->blockSize = blockSize;
    memBlock->maxCount = blockCnt;

    lztSlistInit(&memBlock->blockList);
    while (memBlockStart < memBlockEnd) {
        lztSnodeInit((lztSnode *) memBlockStart);
        lztSListAddLast(&memBlock->blockList, (lztSnode *)memBlockStart);

        memBlockStart += blockSize;
    }
}

/**
 * 等待存储块
 * @param memBlock 等待的存储块
 * @param mem 存储块存储的地址
 * @param waitTicks 当没有存储块时，等待的ticks数，为0时表示永远等待
 * @return 等待结果,tErrorResourceUnavaliable.tErrorNoError,tErrorTimeout
 */
uint32_t lztMemBlockWait (lztMemBlock *memBlock, void **mem, uint32_t waitTicks) {
    uint32_t status = lztTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (lztSlistCount(&memBlock->blockList) > 0) {
        // 如果有的话，取出一个
        *mem = (uint8_t *) lztSListRemoveFirst(&memBlock->blockList);
        lztTaskExitCritical(status);
        return lztErrorNoError;
    } else {
        // 然后将任务插入事件队列中
        lztEventWait(&memBlock->event, currentTask, (void *) 0, lztEventTypeMemBlock, waitTicks);
        lztTaskExitCritical(status);

        // 最后再执行一次事件调度，以便于切换到其它任务
        lztTaskSched();

        // 当切换回来时，从tTask中取出获得的消息
        *mem = currentTask->eventMsg;

        // 取出等待结果
        return currentTask->waitEventResult;
    }
}

/**
 * 获取存储块，如果没有存储块，则立即退回
 * @param memBlock 等待的存储块
 * @param mem 存储块存储的地址
 * @return 获取结果, tErrorResourceUnavaliable.tErrorNoError
 */
uint32_t lztMemBlockNoWaitGet (lztMemBlock *memBlock, void **mem) {
    uint32_t status = lztTaskEnterCritical();

    // 首先检查是否有空闲的存储块
    if (lztSlistCount(&memBlock->blockList) > 0) {
        // 如果有的话，取出一个
        *mem = (uint8_t *) lztSListRemoveFirst(&memBlock->blockList);
        lztTaskExitCritical(status);
        return lztErrorNoError;
    } else {
        // 否则，返回资源不可用
        lztTaskExitCritical(status);
        return lztErrorResourceUnavaliable;
    }
}

/**
 * 通知存储块可用，唤醒等待队列中的一个任务，或者将存储块加入队列中
 * @param memBlock 通知存储块
 * @param memBlock 操作的信号量
 */
void lztMemBlockNotify (lztMemBlock *memBlock, void *mem) {
    uint32_t status = lztTaskEnterCritical();

    // 检查是否有任务等待
    if (lztEventWaitCount(&memBlock->event) > 0) {
        // 如果有的话，则直接唤醒位于队列首部（最先等待）的任务
        lztTask *task = lztEventWakeUp(&memBlock->event, (void *) mem, lztErrorNoError);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        if (task->prio < currentTask->prio) {
            lztTaskSched();
        }
    } else {
        // 如果没有任务等待的话，将存储块插入到队列中
        lztSListAddLast(&memBlock->blockList, (lztSnode *) mem);
    }

    lztTaskExitCritical(status);
}

/**
 * 查询存储控制块的状态信息
 * @param memBlock 存储控制块
 * @param info 状态查询存储的位置
 */
void lztMemBlockGetInfo (lztMemBlock *memBlock, lztMemBlockInfo *info) {
    uint32_t status = lztTaskEnterCritical();

    // 拷贝需要的信息
    info->count = lztSlistCount(&memBlock->blockList);
    info->maxCount = memBlock->maxCount;
    info->blockSize = memBlock->blockSize;
    info->taskCount = lztEventWaitCount(&memBlock->event);

    lztTaskExitCritical(status);
}

/**
 * 销毁存储控制块
 * @param memBlock 需要销毁的存储控制块
 * @return 因销毁该存储控制块而唤醒的任务数量
 */
uint32_t lztMemBlockDestroy (lztMemBlock *memBlock) {
    uint32_t status = lztTaskEnterCritical();

    // 清空事件控制块中的任务
    uint32_t count = lztEventRemoveAll(&memBlock->event, (void *) 0, lztErrorDel);

    lztTaskExitCritical(status);

    // 清空过程中可能有任务就绪，执行一次调度
    if (count > 0) {
        lztTaskSched();
    }
    return count;
}

#endif 
