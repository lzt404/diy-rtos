#include "lztOS.h"
#include "WaveGen.h"
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM

static WaveType currentWaveType = WaveUnknown;
static lztMutex mutex;

/**
 * �������Ӳ����ʼ��
 */
void WaveGenHalInit (void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //��APB2����
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  //��ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;    //PA6 PA7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //Ҫ��Ϊ AF_PP
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_DeInit(TIM3);                           //��TIM3�Ĵ�����Ϊȱʡֵ
    TIM_TimeBaseStructure.TIM_Period = 72 - 1;   //ARR��ֵ,��������
    TIM_TimeBaseStructure.TIM_Prescaler = 36 - 1;      //��Ƶ  CK_INT=2MHz 36��Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);    //��ʼ����ʱ��3

    //ͨ��1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //���ʹ��
    TIM_OCInitStructure.TIM_Pulse = 1 - 1;      //������ȽϼĴ�����ֵ  CRR
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;    //�ߵ�ƽ��Ч
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);     //ͨ����ʼ��
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //Ԥװ��ʹ��
}

/**
 * ��ʼ���������
 */
void WaveGenInit (void) {
    WaveGenHalInit();

    currentWaveType = WaveUnknown;
    lztMutexInit(&mutex);
}

/**
 * ѡ�����������
 * @param type �������
 */
uint32_t WaveSelectType (WaveType type) {
    uint32_t err = 0;

    lztMutexWait(&mutex, 0);

    switch (type) {
        case WaveSquare:
            currentWaveType = type;
            err = 0;
            break;
        default:
            err = 1;
            break;
    }

    lztMutexNotify(&mutex);
}

/**
 * ���÷���������ߵ͵�ƽʱ��
 * @param highMs �ߵ�ƽ����ʱ�䣬msΪ��λ
 * @param lowMs �͵�ƽ����ʱ�䣬msΪ��λ
 */
uint32_t WaveSquareSet (uint32_t highMs, uint32_t lowMs) {
    return 0;
}

/**
 * �����������
 */
uint32_t WaveStartOutput (void) {
    uint32_t err = 0;

    lztMutexWait(&mutex, 0);

    switch (currentWaveType) {
        case WaveSquare:
            TIM3->CNT = 0;
            TIM_Cmd(TIM3, ENABLE);//�򿪶�ʱ��
            err = 0;
        default:
            err = 1;
    }
    lztMutexNotify(&mutex);

    return err;
}

/**
 * ֹͣ�������
 */
void WaveStopOutput (void) {
    lztMutexWait(&mutex, 0);
    switch (currentWaveType) {
        case WaveSquare:
            TIM3->CNT = 0;
            TIM_Cmd(TIM3, DISABLE);
            break;
        default:
            break;
    }
    lztMutexNotify(&mutex);
}
