//存储块

#ifndef TMEMBLOCK_H
#define TMEMBLOCK_H

#include "lztConfig.h"

#if !defined(LZTOS_ENABLE_MEMBLOCK) || LZTOS_ENABLE_MEMBLOCK == 1

#include "lztEvent.h"

typedef struct _lztMemBlock {
    lztEvent event;                   /**< 事件控制块 */

    void *memStart;                 /**< 存储块的首地址 */
    uint32_t blockSize;             /**< 每个存储块的大小 */
    uint32_t maxCount;              /**< 总的存储块的个数 */

    lztSlist blockList;                /**< 存储块列表 */
} lztMemBlock;

typedef struct _lztMemBlockInfo {
    uint32_t count;                 /**< 当前存储块的计数 */
    uint32_t maxCount;              /**< 允许的最大计数 */
    uint32_t blockSize;             /**< 每个存储块的大小 */
    uint32_t taskCount;             /**< 当前等待的任务计数 */
} lztMemBlockInfo;

void lztMemBlockInit (lztMemBlock *memBlock, void *memStart, uint32_t blockSize, uint32_t blockCnt);
uint32_t lztMemBlockWait (lztMemBlock *memBlock, void **mem, uint32_t waitTicks);
uint32_t lztMemBlockNoWaitGet (lztMemBlock *memBlock, void **mem);
void lztMemBlockNotify (lztMemBlock *memBlock, void *mem);
void lztMemBlockGetInfo (lztMemBlock *memBlock, lztMemBlockInfo *info);
uint32_t lztMemBlockDestroy (lztMemBlock *memBlock);

#endif

#endif

