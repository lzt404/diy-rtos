#include "lztOS.h"
#include "app.h"
#include "hal.h"

static lztTask task1;                     // 任务1结构
static lztTask task2;                     // 任务2结构
static lztTask task3;                     // 任务3结构
static lztTask task4;                     // 任务4结构

static lztTaskStack task1Env[TASK1_ENV_SIZE];     // 任务1的堆栈空间
static lztTaskStack task2Env[TASK2_ENV_SIZE];     // 任务2的堆栈空间
static lztTaskStack task3Env[TASK3_ENV_SIZE];     // 任务3的堆栈空间
static lztTaskStack task4Env[TASK4_ENV_SIZE];     // 任务4的堆栈空间

int task1Flag;           // 用于指示任务运行状态的标志变量
int task2Flag;           // 用于指示任务运行状态的标志变量
int task3Flag;           // 用于指示任务运行状态的标志变量
int task4Flag;           // 用于指示任务运行状态的标志变量

void waitEvent (void) {
    xprintf("Wait event!\n");

    lztTaskDelay(1);
}

void processEvent (void) {
    xprintf("Process event!\n");
}

void notifyOtherTask (void) {
    xprintf("Notify other task!\n");
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task1Entry (void *param) {
    for (;;) {
//        waitEvent();
//        processEvent();
        //notifyOtherTask();
		//	xprintf("任务1正在运行\n");
        printf("任务2正在运行\n");
//			lztTaskDelay(2);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task2Entry (void *param) {
    for (;;) {
        //xprintf("任务2第%d次运行\n", i);
        //xprintf("任务2正在运行\n");
        printf("任务2正在运行\n");
//        task2Flag = 1;
//        lztTaskDelay(1);
//        task2Flag = 0;
//        lztTaskDelay(1);
    }
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task3Entry (void *param) {
    for (;;) {
        task3Flag = 1;
        lztTaskDelay(1);
        task3Flag = 0;
        lztTaskDelay(1);
    }
}


/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
void task4Entry (void *param) {
    for (;;) {
        task4Flag = 1;
        lztTaskDelay(1);
        task4Flag = 0;
        lztTaskDelay(1);
    }
}

/**
 * App的初始化
 */
void lztInitApp (void) {
    halInit();

    lztTaskInit(&task1, task1Entry, (void *) 0x0, TASK1_PRIO, task1Env, sizeof(task1Env));
    lztTaskInit(&task2, task2Entry, (void *) 0x0, TASK2_PRIO, task2Env, sizeof(task2Env));
    lztTaskInit(&task3, task3Entry, (void *) 0x0, TASK3_PRIO, task3Env, sizeof(task3Env));
    lztTaskInit(&task4, task4Entry, (void *) 0x0, TASK4_PRIO, task4Env, sizeof(task4Env));
}

