//数据结构实现
#ifndef LZTLIB_H
#define LZTLIB_H

#include <stdint.h>

/**
 * @defgroup 位图结构 位图结构
 * @{
 */
// 位图类型
typedef struct {
    uint32_t bitmap;       /**< 该位图只支持最大32位，如果需要的可扩充至更多位 */
} lztBitmap;

void lztBitmapInit (lztBitmap *bitmap);
uint32_t lztBitmapPosCount (void);
void lztBitmapSet (lztBitmap *bitmap, uint32_t pos);
void lztBitmapClear (lztBitmap *bitmap, uint32_t pos);
uint32_t lztBitmapGetFirstSet (lztBitmap *bitmap);

/** @} */ // 模块结尾

/**
 * @defgroup 双向链表 双向链表
 * @{
 */
// lztOS链表的结点类型
typedef struct _lztNode {
    struct _lztNode *preNode;             /**< 前一结点 */
    struct _lztNode *nextNode;            /**< 后一结点 */
} lztNode;

void lztNodeInit (lztNode *node);

// lztOS链表类型
typedef struct _tList {
    lztNode headNode;
    uint32_t nodeCount;
} lztList;

#define lztNodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name)

/**
 * 初始化链表
 * @param list 等待初始化的链表
 */
void lztListInit (lztList *list);

/**
 * 返回链表中结点的数量
 * @param list 查询的链表
 * @return 结点数量
 */
uint32_t lztListCount (lztList *list);

/**
 * 返回链表中首个结点
 * @param list 查询的链表
 * @return 首个结点，如果没有，返回0
 */
lztNode *lztListFirst (lztList *list);

/**
 * 返回链表中最后结点
 * @param list 查询的链表
 * @return 最后结点，如果没有，返回0
 */
lztNode *lztListLast (lztList *list);

/**
 * 返回链表中指定结点的前一结点
 * @param list 查询的链表
 * @param node 查询的结点
 * @return 前一结点，如果没有，返回0
 */
lztNode *lztListPre (lztList *list, lztNode *node);

/**
 * 返回链表中指定结点的后一结点
 * @param list 查询的链表
 * @param node 查询的结点
 * @return 后一结点，如果没有，返回0
 */
lztNode *lztListNext (lztList *list, lztNode *node);

/**
 * 清空链表中的所有结点
 * @param list 等待清空的链表
 */
void lztListRemoveAll (lztList *list);

/**
 * 将指定结点插入到链表开始处
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void lztListAddFirst (lztList *list, lztNode *node);

/**
 * 将指定结点插入到链表最后
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void lztListAddLast (lztList *list, lztNode *node);

/**
 * 移除链表的第一个结点
 * @param list 操作的链表
 * @return 移除的结点，如果没有，返回0
 */
lztNode *lztListRemoveFirst (lztList *list);

/**
 * 将指定结点插入到某个结点之后
 * @param list 操作的链表
 * @param nodeAfter 参考结点
 * @param nodeToInsert 等待插入的结点
 */
void lztListInsertAfter (lztList *list, lztNode *nodeAfter, lztNode *nodeToInsert);

/**
 * 移除链表中指定结点
 * @param list 操作的链表
 * @param node 等待队列的结点
 */
void lztListRemove (lztList *list, lztNode *node);

/** @} */

/**
 * @defgroup 单向链表 单向链表
 * @{
 */
/**
 * 单向链表结点
 */
typedef struct _lztSnode {
    struct _lztSnode * next;                  /**<  后一结点 */
}lztSnode;

void lztSnodeInit (lztSnode * snode);

/**
 * 单向链表
 */
typedef struct _lztSlist {
    lztSnode * firstNode;                     /**<  第一个结点 */
    lztSnode * lastNode;                      /**<  最后一个结点 */
    uint32_t nodeCount;                     /**<  总的结点数量 */
}lztSlist;

void lztSlistInit (lztSlist * slist);
uint32_t lztSlistCount (lztSlist * slist);
lztSnode * lztSlistFirst (lztSlist * slist);
lztSnode * lztSlistLast (lztSlist * slist);
void lztSListAddFirst (lztSlist *slist, lztSnode * snode);
void lztSListAddLast (lztSlist *slist, lztSnode * snode);
lztSnode * lztSListRemoveFirst (lztSlist * slist);

#endif
