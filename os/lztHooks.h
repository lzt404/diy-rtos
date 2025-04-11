//Hooks扩展 Hooks扩展

#ifndef LZTHOOKS_H
#define LZTHOOKS_H

#include "lztOS.h"

/**
 * cpu空闲时的hooks
 */
void lztHooksCpuIdle (void);

/**
 * 时钟节拍Hooks
 */
void lztHooksSysTick (void);

/**
 * 任务切换hooks
 * @param from 从哪个任务开始切换
 * @param to 切换至哪个任务
 */
void lztHooksTaskSwitch (lztTask *from, lztTask *to);

/**
 * 任务初始化的Hooks
 * @param task 等待初始化的任务
 */
void lztHooksTaskInit (lztTask *task);

#endif
