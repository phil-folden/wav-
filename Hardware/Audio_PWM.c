#include <stm32f10x.h> 
#include "Audio_PWM.h"

void Audio_Timer_Init(void){
    TIM_InternalClockConfig(TIM1);

    //配置TIM1的基本参数，使其能够产生PWM信号
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = AUDIO_TIM1_ARR;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = AUDIO_TIM1_RCR;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = AUDIO_PWM_MID_VALUE;  // 占空比50%
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    //初始化第一个PWM
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    //初始化第二个PWM
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);

    //预装载
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    TIM_DMAConfig(
        TIM1,
        TIM_DMABase_CCR1,
        TIM_DMABurstLength_2Transfers
    );

    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);

    TIM_Cmd(TIM1, ENABLE);

    TIM_SetCounter(TIM1, 0);
    TIM_GenerateEvent(TIM1, TIM_EventSource_Update);
    TIM_ClearFlag(TIM1, TIM_EventSource_Update);
}

void Audio_GPIO_Init(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_TIM1  | 
                           RCC_APB2Periph_AFIO,
                           ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

uint8_t PCM16_To_PWM(int16_t sample){
    int32_t value = ((int32_t)sample + 32768) * AUDIO_TIM1_ARR;

    value /= 65535;

    if(value < 0){
        value = 0;
    }
    else if(value > AUDIO_TIM1_ARR){
        value = AUDIO_TIM1_ARR;
    }

    return (uint8_t)value;
}

void Audio_PWM_SetSample(uint16_t sample){
    if(sample > 255){
        sample = 255;
    }
    TIM_SetCompare1(TIM2, sample);
}
