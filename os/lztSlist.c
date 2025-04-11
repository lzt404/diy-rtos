//单向链表

#include "lztLib.h"

/**
 * 初始化单链表结点
 * @param snode 单链表结点
 */
void lztSnodeInit (lztSnode * snode) {
    snode->next = snode;
}

/**
 * 初始化单链表
 * @param slist 单链表
 */
void lztSlistInit (lztSlist * slist) {
    slist->firstNode = (lztSnode *)0;
    slist->lastNode = (lztSnode *)0;
    slist->nodeCount = 0;
}

/**
 * 获取单链表结点数量
 * @param slist 查询的单链表
 * @return 结点数量
 */
uint32_t lztSlistCount (lztSlist * slist) {
    return slist->nodeCount;
}

/**
 * 获取单链表的第一个结点
 * @param slist 查询的单链表
 * @return 第一个结点，如果没有，返回0
 */
lztSnode * lztSlistFirst (lztSlist * slist) {
    return slist->firstNode;
}

/**
 * 获取单链表的最后一个结点
 * @param slist 查询的单链表
 * @return 最后一个结点，如果没有，返回0
 */
lztSnode * tSlistLast (lztSlist * slist) {
    return slist->lastNode;
}

/**
 * 将结点添加到链表表头
 * @param slist 操作的链表
 * @param snode 待插入的结点
 */
void lztSListAddFirst (lztSlist *slist, lztSnode * snode) {
    if (slist->nodeCount == 0) {
        slist->firstNode = snode;
        slist->lastNode = snode;
        slist->nodeCount = 1;
    } else {
        snode->next = slist->firstNode;
        slist->firstNode = snode;
        slist->nodeCount++;
    }
}

/**
 * 将结点插入到链表尾部
 * @param slist 操作的链表
 * @param snode 待插入的结点
 */
void lztSListAddLast (lztSlist *slist, lztSnode * snode) {
    if (slist->nodeCount == 0) {
        slist->firstNode = snode;
        slist->lastNode = snode;
        slist->nodeCount = 1;
    } else {
        slist->lastNode->next = snode;
        snode->next = snode;
        slist->lastNode = snode;
        slist->nodeCount++;
    }
}

/**
 * 移除链表的首个结点
 * @param slist 操作的链表
 * @return 移除的结点，如果没有，返回0
 */
lztSnode * lztSListRemoveFirst (lztSlist * slist) {
    switch (slist->nodeCount) {
        case 0:
            return (lztSnode *)0;
        case 1: {
            lztSnode * removeNode = slist->firstNode;
            removeNode->next = removeNode;

            slist->firstNode = (lztSnode *)0;
            slist->lastNode = (lztSnode *)0;
            slist->nodeCount = 0;
            return removeNode;
        }
        default: {
            lztSnode * removeNode = slist->firstNode;

            slist->firstNode = removeNode->next;
            removeNode->next = removeNode;
            slist->nodeCount--;
            return removeNode;
        }

    }
}
/** @} */

