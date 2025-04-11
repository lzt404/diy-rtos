/**位图数据结构
 主要用于实现os中快速优先级查找功能。位图中包含一连串的位，每个位均有编号
 * 编号从0开始增大。每个位均可置1或者清0.
 */
#include "lztLib.h"

/**
 * 初始化位图结构
 * @param bitmap 待初始化的位图结构体
 */
void lztBitmapInit (lztBitmap *bitmap) {
    bitmap->bitmap = 0;
}

/**
 * 返回最大支持的位图位置序号，从0开始
 * @return 最大支持的位图位置序号
 */
uint32_t lztBitmapPosCount (void) {
    return 32;
}

/**
 * 将位图中指定的位置置1
 * @param bitmap 等待设置的位图
 * @param pos 需要设置的位置序号，从0开始
 */
void lztBitmapSet (lztBitmap *bitmap, uint32_t pos) {
    bitmap->bitmap |= 1 << pos;
}

/**
 * 将位图中指定的位置清0
 * @param bitmap 等待清0的位图
 * @param pos 需要清0的位置序号，从0开始
 */
void lztBitmapClear (lztBitmap *bitmap, uint32_t pos) {
    bitmap->bitmap &= ~(1 << pos);
}

/**
 * 获取位置中从位置0开始数起，第1个置1的位置
 * @param bitmap 待查找的位图结构
 * @return 第1个置1的位置序号。如果没有位置1，返回tBitmapPosCount()的值
 */
uint32_t lztBitmapGetFirstSet (lztBitmap *bitmap) {
    // 快速查找表，用于实现8位宽数据中第1个置1的位置序号
    static const uint8_t quickFindTable[] = {
            /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 10 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 20 */    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 30 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 40 */    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 50 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 60 */    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 70 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 80 */    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 90 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* A0 */    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* B0 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* C0 */    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* D0 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* E0 */    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* F0 */    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
    };

    // 将32位依次折成多个8位，利用查找表进行快速查找
    if (bitmap->bitmap & 0xff) {
        return quickFindTable[(bitmap->bitmap >>  0) & 0xff] + 0;
    } else if (bitmap->bitmap & 0xff00) {
        return quickFindTable[(bitmap->bitmap >>  8) & 0xff] + 8;
    } else if (bitmap->bitmap & 0xff0000) {
        return quickFindTable[(bitmap->bitmap >> 16) & 0xff] + 16;
    } else if (bitmap->bitmap & 0xFF000000) {
        return quickFindTable[(bitmap->bitmap >> 24) & 0xFF] + 24;
    } else {
        return lztBitmapPosCount();
    }
}

/** @} */
