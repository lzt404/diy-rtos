//双向链表 双向链表
#include "lztLib.h"

/**
 * 初始化结构
 * @param node 等待初始化的结点
 */
void lztNodeInit (lztNode *node) {
    node->nextNode = node;
    node->preNode = node;
}

// 以下是简化代码编写添加的宏
#define firstNode   headNode.nextNode
#define lastNode    headNode.preNode

/**
 * 初始化链表
 * @param list 等待初始化的链表
 */
void lztListInit (lztList *list) {
    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/**
 * 返回链表中结点的数量
 * @param list 查询的链表
 * @return 结点数量
 */
uint32_t lztListCount (lztList *list) {
    return list->nodeCount;
}

/**
 * 返回链表中首个结点
 * @param list 查询的链表
 * @return 首个结点，如果没有，返回0
 */
lztNode *lztListFirst (lztList *list) {
    lztNode *node = (lztNode *) 0;

    if (list->nodeCount != 0) {
        node = list->firstNode;
    }
    return node;
}

/**
 * 返回链表中最后结点
 * @param list 查询的链表
 * @return 最后结点，如果没有，返回0
 */
lztNode *lztListLast (lztList *list) {
    lztNode *node = (lztNode *) 0;

    if (list->nodeCount != 0) {
        node = list->lastNode;
    }
    return node;
}

/**
 * 返回链表中指定结点的前一结点
 * @param list 查询的链表
 * @param node 查询的结点
 * @return 前一结点，如果没有，返回0
 */
lztNode *lztListPre (lztList *list, lztNode *node) {
    if ((node->preNode == &list->headNode) || (node->preNode == node)) {
        return (lztNode *) 0;
    } else {
        return node->preNode;
    }
}

/**
 * 返回链表中指定结点的后一结点
 * @param list 查询的链表
 * @param node 查询的结点
 * @return 后一结点，如果没有，返回0
 */
lztNode *lztListNext (lztList *list, lztNode *node) {
    if ((node->nextNode == &list->headNode) || (node->nextNode == &list->headNode)) {
        return (lztNode *) 0;
    } else {
        return node->nextNode;
    }
}

/**
 * 清空链表中的所有结点
 * @param list 等待清空的链表
 */
void lztListRemoveAll (lztList *list) {
    uint32_t count;
    lztNode *nextNode;

    // 遍历所有的结点
    nextNode = list->firstNode;
    for (count = list->nodeCount; count != 0; count--) {
        // 先纪录下当前结点，和下一个结点
        // 必须纪录下一结点位置，因为在后面的代码中当前结点的next会被重置
        lztNode *currentNode = nextNode;
        nextNode = nextNode->nextNode;

        // 重置结点自己的信息
        currentNode->nextNode = currentNode;
        currentNode->preNode = currentNode;
    }

    list->firstNode = &(list->headNode);
    list->lastNode = &(list->headNode);
    list->nodeCount = 0;
}

/**
 * 将指定结点插入到链表开始处
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void lztListAddFirst (lztList *list, lztNode *node) {
    node->preNode = list->firstNode->preNode;
    node->nextNode = list->firstNode;

    list->firstNode->preNode = node;
    list->firstNode = node;
    list->nodeCount++;
}

/**
 * 将指定结点插入到链表最后
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void lztListAddLast (lztList *list, lztNode *node) {
    node->nextNode = &(list->headNode);
    node->preNode = list->lastNode;

    list->lastNode->nextNode = node;
    list->lastNode = node;
    list->nodeCount++;
}

/**
 * 移除链表的第一个结点
 * @param list 操作的链表
 * @return 移除的结点，如果没有，返回0
 */
lztNode *lztListRemoveFirst (lztList *list) {
    lztNode *node = (lztNode *) 0;

    if (list->nodeCount != 0) {
        node = list->firstNode;

        node->nextNode->preNode = &(list->headNode);
        list->firstNode = node->nextNode;
        list->nodeCount--;

        // 重置结点自己的信息
        node->nextNode = node;
        node->preNode = node;
    }
    return node;
}

/**
 * 将指定结点插入到某个结点之后
 * @param list 操作的链表
 * @param nodeAfter 参考结点
 * @param nodeToInsert 等待插入的结点
 */
void lztListInsertAfter (lztList *list, lztNode *nodeAfter, lztNode *nodeToInsert) {
    nodeToInsert->preNode = nodeAfter;
    nodeToInsert->nextNode = nodeAfter->nextNode;

    nodeAfter->nextNode->preNode = nodeToInsert;
    nodeAfter->nextNode = nodeToInsert;

    list->nodeCount++;
}

/**
 * 移除链表中指定结点
 * @param list 操作的链表
 * @param node 等待队列的结点
 */
void lztListRemove (lztList *list, lztNode *node) {
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    list->nodeCount--;

    // 重置结点自己的信息
    node->nextNode = node;
    node->preNode = node;
}
