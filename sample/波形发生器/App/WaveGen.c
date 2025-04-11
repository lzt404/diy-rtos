#include "lztOS.h"
#include "WaveGen.h"
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM

static WaveType currentWaveType = WaveUnknown;
static lztMutex mutex;

/**
 * 波形输出硬件初始化
 */
void WaveGenHalInit (void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //打开APB2外设
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  //开时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;    //PA6 PA7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //要改为 AF_PP
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_DeInit(TIM3);                           //将TIM3寄存器设为缺省值
    TIM_TimeBaseStructure.TIM_Period = 72 - 1;   //ARR的值,计数周期
    TIM_TimeBaseStructure.TIM_Prescaler = 36 - 1;      //分频  CK_INT=2MHz 36分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);    //初始化定时器3

    //通道1
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //输出使能
    TIM_OCInitStructure.TIM_Pulse = 1 - 1;      //待捕获比较寄存器的值  CRR
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;    //高电平有效
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);     //通道初始化
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //预装载使能
}

/**
 * 初始化波形输出
 */
void WaveGenInit (void) {
    WaveGenHalInit();

    currentWaveType = WaveUnknown;
    lztMutexInit(&mutex);
}

/**
 * 选择波形输出类型
 * @param type 输出类型
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
 * 设置方波的输出高低电平时间
 * @param highMs 高电平持续时间，ms为单位
 * @param lowMs 低电平持续时间，ms为单位
 */
uint32_t WaveSquareSet (uint32_t highMs, uint32_t lowMs) {
    return 0;
}

/**
 * 启动波形输出
 */
uint32_t WaveStartOutput (void) {
    uint32_t err = 0;

    lztMutexWait(&mutex, 0);

    switch (currentWaveType) {
        case WaveSquare:
            TIM3->CNT = 0;
            TIM_Cmd(TIM3, ENABLE);//打开定时器
            err = 0;
        default:
            err = 1;
    }
    lztMutexNotify(&mutex);

    return err;
}

/**
 * 停止波形输出
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
