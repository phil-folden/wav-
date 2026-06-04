#include <stm32f10x.h>
#include "OLED.h"
#include "Scene_Manager.h"
#include "Serial.h"
volatile int8_t rotate_index = 0;

#define ROTATE_STEPS_PER_CLICK     4

/*
 * 上一次稳定的 A/B 状态
 */
static volatile uint8_t rotate_last_ab = 0;

/*
 * 合法跳变累计值
 */
static volatile int8_t rotate_step_acc = 0;

static uint8_t Rotate_ReadAB(void)
{
    uint8_t a;
    uint8_t b;

    a = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) ? 1 : 0;
    b = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) ? 1 : 0;

    /*
     * bit0 = A
     * bit1 = B
     */
    return (uint8_t)((a << 0) | (b << 1));
}

static uint8_t Rotate_EXTI_Process(void){
    uint8_t now_ab;
    int8_t step;
    uint8_t transition;
    now_ab = Rotate_ReadAB();
    if(now_ab == rotate_last_ab)
    {
        return 0;
    }

    transition = (rotate_last_ab << 2) | now_ab;
    switch(transition){
        case 0x01:
        case 0x07:
        case 0x0E:
        case 0x08:
            step = 1;
            break;
        case 0x02:
        case 0x0B:
        case 0x0D:
        case 0x04:
            step = -1;
            break;
        default:
            rotate_step_acc = 0;
            rotate_last_ab = now_ab;
            return 0;
    }
    rotate_last_ab = now_ab;

    if((rotate_step_acc > 0 && step < 0) ||
    (rotate_step_acc < 0 && step > 0))
    {
        rotate_step_acc = 0;
    }

    rotate_step_acc += step;

    if(rotate_step_acc >= ROTATE_STEPS_PER_CLICK)
    {
        rotate_step_acc = 0;
        rotate_index = 1;
        return 1;
    }
    else if(rotate_step_acc <= -ROTATE_STEPS_PER_CLICK)
    {
        rotate_step_acc = 0;
        rotate_index = -1;
        return 1;
    }
    return 0;
}

void Rotate_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitTypeDef Rotate_GPIO;
    Rotate_GPIO.GPIO_Mode = GPIO_Mode_IPU;
    Rotate_GPIO.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    Rotate_GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &Rotate_GPIO);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

    EXTI_InitTypeDef Rotate_Exti;
    Rotate_Exti.EXTI_Line = EXTI_Line3 | EXTI_Line4;
    Rotate_Exti.EXTI_LineCmd = ENABLE;
    Rotate_Exti.EXTI_Mode = EXTI_Mode_Interrupt;
    Rotate_Exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&Rotate_Exti);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef Rotate_NVIC;
	Rotate_NVIC.NVIC_IRQChannel = EXTI3_IRQn;
	Rotate_NVIC.NVIC_IRQChannelCmd = ENABLE;
	Rotate_NVIC.NVIC_IRQChannelPreemptionPriority = 1;
	Rotate_NVIC.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&Rotate_NVIC);
	
	Rotate_NVIC.NVIC_IRQChannel = EXTI4_IRQn;
	Rotate_NVIC.NVIC_IRQChannelCmd = ENABLE;
	Rotate_NVIC.NVIC_IRQChannelPreemptionPriority = 1;
	Rotate_NVIC.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&Rotate_NVIC);
    rotate_last_ab = Rotate_ReadAB();
    rotate_step_acc = 0;
    rotate_index = 0;
}

void EXTI3_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
        if(Rotate_EXTI_Process()){
            need_flash = 1;
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line4) == SET)
	{
        if(Rotate_EXTI_Process()){
            need_flash = 1;
        }
        EXTI_ClearITPendingBit(EXTI_Line4);
	}		
}

int8_t get_Rotate_count(void){
    int8_t count;

    __disable_irq();
    if(rotate_index > 0){
        count = 1;
    }
    else if(rotate_index < 0){
        count = -1;
    }
    else{
        count = 0;
    }
    rotate_index = 0;
    __enable_irq();

    return count;
}
