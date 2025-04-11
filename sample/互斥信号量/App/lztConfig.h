//lztOS全局配置
#ifndef LZTCONFIG_H
#define LZTCONFIG_H

#define LZTOS_PRO_COUNT                32                        // lztOS任务的优先级序号
#define LZTOS_SLICE_MAX                10                        // 每个任务最大运行的时间片计数

#define LZTOS_IDLETASK_STACK_SIZE        1024                    // 空闲任务的堆栈单元数
#define LZTOS_TIMERTASK_STACK_SIZE        1024                    // 定时器任务的堆栈单元数
#define LZTOS_TIMERTASK_PRIO           1                       // 定时器任务的优先级

#define LZTOS_SYSTICK_MS               10                      // 时钟节拍的周期，以ms为单位

// 内核功能裁剪部分
#define LZTOS_ENABLE_SEM               1                       // 是否使能信号量
#define LZTOS_ENABLE_MUTEX             1                       // 是否使能互斥信号量
#define LZTOS_ENABLE_FLAGGROUP         1                       // 是否使能事件标志组
#define LZTOS_ENABLE_MBOX              1                       // 是否使能邮箱
#define LZTOS_ENABLE_MEMBLOCK          1                       // 是否使能存储块
#define LZTOS_ENABLE_TIMER             1                       // 是否使能定时器
#define LZTOS_ENABLE_CPUUSAGE_STAT     0                       // 是否使能CPU使用率统计
#define LZTOS_ENABLE_HOOKS             1                       // 是否使能Hooks

#endif /* TCONFIG_H */
