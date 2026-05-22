#include "stm32f10x.h"
#include "Key.h"

uint8_t key_count = 0;

void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_Key;
    GPIO_Key.GPIO_Pin = GPIO_Pin_1;
    GPIO_Key.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Key.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_Key);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);

    EXTI_InitTypeDef Key_EXTI;
    Key_EXTI.EXTI_Line = EXTI_Line1;
    Key_EXTI.EXTI_LineCmd = ENABLE;
    Key_EXTI.EXTI_Mode = EXTI_Mode_Interrupt;
    Key_EXTI.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&Key_EXTI);

    EXTI_ClearITPendingBit(EXTI_Line1);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef KEY_NVIC;
    KEY_NVIC.NVIC_IRQChannel = EXTI1_IRQn;
    KEY_NVIC.NVIC_IRQChannelCmd = ENABLE;
    KEY_NVIC.NVIC_IRQChannelPreemptionPriority = 2;
    KEY_NVIC.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&KEY_NVIC);
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) == SET)
    {
        key_count = 1;

        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}