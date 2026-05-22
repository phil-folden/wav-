#include <stm32f10x.h>
#include "Audio_Timer.h"

void Init_Audio_Timer(void){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 1632;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_DMACmd(TIM3, TIM_DMA_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void Audio_PWM_PA0_Close(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*
     * 1. 先关闭 TIM2_CH1 输出通道
     */
    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);

    /*
     * 2. 停止 TIM2
     */
    TIM_Cmd(TIM2, DISABLE);

    /*
     * 3. 把 PA0 从 TIM2_CH1 复用功能改回普通 GPIO 输出
     */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*
     * 4. PA0 输出低电平
     */
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}