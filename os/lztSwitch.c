//内核核心

#include "lztOS.h"
#include "stm32f10x.h"

// 在任务切换中，主要依赖了PendSV进行切换。PendSV其中的一个很重要的作用便是用于支持RTOS的任务切换。
// 实现方法为：
// 1、首先将PendSV的中断优先配置为最低。这样只有在其它所有中断完成后，才会触发该中断；
//    实现方法为：向NVIC_SYSPRI2写NVIC_PENDSV_PRI
// 2、在需要中断切换时，设置挂起位为1，手动触发。这样，当没有其它中断发生时，将会引发PendSV中断。
//    实现方法为：向NVIC_INT_CTRL写NVIC_PENDSVSET
// 3、在PendSV中，执行任务切换操作。
#define NVIC_INT_CTRL       0xE000ED04      // 中断控制及状态寄存器
#define NVIC_PENDSVSET      0x10000000      // 触发软件中断的值
#define NVIC_SYSPRI2        0xE000ED22      // 系统优先级寄存器
#define NVIC_PENDSV_PRI     0x000000FF      // 配置优先级

#define MEM32(addr)         *(volatile unsigned long *)(addr)
#define MEM8(addr)          *(volatile unsigned char *)(addr)

/**
 * 进入临界区
 * @return 进入之前的临界区状态值
 */
uint32_t lztTaskEnterCritical (void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();        // CPSID I
    return primask;
}

/**
 * 退出临界区,恢复之前的临界区状态
 * @param status 进入临界区之前的CPU
 * @return 进入临界区之前的临界区状态值
 */
void lztTaskExitCritical (uint32_t status) {
    __set_PRIMASK(status);
}

/**
 * PendSV异常处理函数，用于任务切换
 */
__asm void PendSV_Handler (void) { 
    IMPORT saveAndLoadStackAddr         // 汇编代码中导入外部符号，类似C中的extern声明
                                        // 进入异常前，硬件会自动将R0~R3等内核寄存器保存到PSP堆栈中
    MRS     R0, PSP                     // 获取PSP，其当前指向正在运行任务的栈中
    STMDB   R0!, {R4-R11}               // 将未自动保存到任务栈中的R4-R11寄存器保存到任务栈

    BL      saveAndLoadStackAddr        // 调用saveAndLoadStackAddr,在这里负责切换堆栈

    LDMIA   R0!, {R4-R11}               // 从将要运行的任务栈中，加载R4-R11的值到内核寄存器
    MSR     PSP, R0                     // 之后，调用PSP的栈指针为将要运行的任务本
    
    MOV     LR, #0xFFFFFFFD             // 指明退出异常时，使用PSP堆栈。0xFFFFFFFD含义特殊
    BX      LR                          // 如需了解含义，请查资料。之后退出异常，硬件自动弹出R0~R3等寄存器
}

/**
 * 保存前一任务栈地址，获取下一任务栈地址
 * @param stackAddr 前一任务栈地址
 * @return 下一要运行的任务栈地址
 */
uint32_t saveAndLoadStackAddr (uint32_t stackAddr) {
    if (currentTask != (lztTask *)0) {
        currentTask->stack = (uint32_t *)stackAddr;
    }
    currentTask = nextTask;
    return (uint32_t)currentTask->stack;
}

/**
 * 在启动lztOS时，调用该函数，将切换至第一个任务运行
 */
void lztTaskRunFirst () {
    __set_PSP(__get_MSP());

    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;   // 向NVIC_SYSPRI2写NVIC_PENDSV_PRI，设置其为最低优先级

    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;    // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV

    // 可以看到，这个函数是没有返回
    // 这是因为，一旦触发PendSV后，将会在PendSV后立即进行任务切换，切换至第1个任务运行
    // 此后，lztOS将负责管理所有任务的运行，永远不会返回到该函数运行
}

/**
 * 进行一次任务切换，lztOS会预先配置好currentTask和nextTask, 然后调用该函数，切换至nextTask运行
 */
void lztTaskSwitch () {
    // 和tTaskRunFirst, 这个函数会在某个任务中调用，然后触发PendSV切换至其它任务
    // 之后的某个时候，将会再次切换到该任务运行，此时，开始运行该行代码, 返回到
    // tTaskSwitch调用处继续往下运行
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}

/** @} */
