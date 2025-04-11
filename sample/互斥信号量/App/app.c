#include "lztOS.h"
#include "app.h"
#include "hal.h"

static lztTask task1;                     // ����1�ṹ
static lztTask task2;                     // ����2�ṹ
static lztTask task3;                     // ����3�ṹ
static lztTask task4;                     // ����4�ṹ

static lztTaskStack task1Env[TASK1_ENV_SIZE];     // ����1�Ķ�ջ�ռ�
static lztTaskStack task2Env[TASK2_ENV_SIZE];     // ����2�Ķ�ջ�ռ�
static lztTaskStack task3Env[TASK3_ENV_SIZE];     // ����3�Ķ�ջ�ռ�
static lztTaskStack task4Env[TASK4_ENV_SIZE];     // ����4�Ķ�ջ�ռ�

int task1Flag;           // ����ָʾ��������״̬�ı�־����
int task2Flag;           // ����ָʾ��������״̬�ı�־����
int task3Flag;           // ����ָʾ��������״̬�ı�־����
int task4Flag;           // ����ָʾ��������״̬�ı�־����

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
 * ��������д���
 * @param param �����ʼ���в���
 */
void task1Entry (void *param) {
    for (;;) {
//        waitEvent();
//        processEvent();
        //notifyOtherTask();
		//	xprintf("����1��������\n");
        printf("����2��������\n");
//			lztTaskDelay(2);
    }
}

/**
 * ��������д���
 * @param param �����ʼ���в���
 */
void task2Entry (void *param) {
    for (;;) {
        //xprintf("����2��%d������\n", i);
        //xprintf("����2��������\n");
        printf("����2��������\n");
//        task2Flag = 1;
//        lztTaskDelay(1);
//        task2Flag = 0;
//        lztTaskDelay(1);
    }
}

/**
 * ��������д���
 * @param param �����ʼ���в���
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
 * ��������д���
 * @param param �����ʼ���в���
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
 * App�ĳ�ʼ��
 */
void lztInitApp (void) {
    halInit();

    lztTaskInit(&task1, task1Entry, (void *) 0x0, TASK1_PRIO, task1Env, sizeof(task1Env));
    lztTaskInit(&task2, task2Entry, (void *) 0x0, TASK2_PRIO, task2Env, sizeof(task2Env));
    lztTaskInit(&task3, task3Entry, (void *) 0x0, TASK3_PRIO, task3Env, sizeof(task3Env));
    lztTaskInit(&task4, task4Entry, (void *) 0x0, TASK4_PRIO, task4Env, sizeof(task4Env));
}

