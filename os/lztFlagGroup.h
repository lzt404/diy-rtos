//事件标志组 事件标志组

#ifndef TFLAGGROUP_H
#define TFLAGGROUP_H

#include "lztConfig.h"

#if !defined(LZTOS_ENABLE_FLAGGROUP) || LZTOS_ENABLE_FLAGGROUP == 1

#include "lztEvent.h"

// 事件标志组结构
typedef struct _lztFlagGroup {
    lztEvent event;                           /**< 事件控制块 */
    uint32_t flags;                         /**< 当前事件标志 */
} lztFlagGroup;

// 事件标志组查询信息
typedef struct _lztFlagGroupInfo {
    uint32_t flags;                         /**< 当前的事件标志 */
    uint32_t taskCount;                     /**< 当前等待的任务计数 */
} lztFlagGroupInfo;

#define LZTFLAGGROUP_CLEAR            (0x0 << 0)
#define LZTFLAGGROUP_SET              (0x1 << 0)
#define LZTFLAGGROUP_ANY              (0x0 << 1)
#define LZTFLAGGROUP_ALL              (0x1 << 1)

#define LZTFLAGGROUP_SET_ALL          (LZTFLAGGROUP_SET | LZTFLAGGROUP_ALL)
#define LZTFLAGGROUP_SET_ANY          (LZTFLAGGROUP_SET | LZTFLAGGROUP_ANY)
#define LZTFLAGGROUP_CLEAR_ALL        (LZTFLAGGROUP_CLEAR | LZTFLAGGROUP_ALL)
#define LZTFLAGGROUP_CLEAR_ANY        (LZTFLAGGROUP_CLEAR | LZTFLAGGROUP_ANY)

#define LZTFLAGGROUP_CONSUME        (0x1 << 7)

void lztFlagGroupInit (lztFlagGroup *flagGroup, uint32_t flags);
uint32_t lztFlagGroupWait (lztFlagGroup *flagGroup, uint32_t waitType, uint32_t requestFlag,
                         uint32_t *resultFlag, uint32_t waitTicks);
uint32_t lztFlagGroupNoWaitGet (lztFlagGroup *flagGroup, uint32_t waitType, uint32_t requstFlag, uint32_t *resultFlag);
void lztFlagGroupNotify (lztFlagGroup *flagGroup, uint8_t isSet, uint32_t flags);
void lztFlagGroupGetInfo (lztFlagGroup *flagGroup, lztFlagGroupInfo *info);
uint32_t lztFlagGroupDestroy (lztFlagGroup *flagGroup);

#endif


#endif
