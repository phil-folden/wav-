#include "stm32f10x.h"
#include "Key.h" 

#define KEY_SCANNED_MS 5
#define KEY_SCANNED_COUNT 4

#define KEY_PRESSED_TASK 0
#define KEY_RELEASED_TASK 1

extern volatile uint32_t sys_ms;

static uint32_t key_last_scanned_time = 0;
static uint8_t key_scanned_count = 0;

static uint8_t key_last_raw = KEY_RELEASED_TASK;
static uint8_t key_stable_state = KEY_RELEASED_TASK;
static Key_Event key_event = KEY_EVENT_NONE;

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
}

void Key_Scanned(void){
    uint8_t raw;
    //如果还没到5ms，则啥也不发生
    if((sys_ms - key_last_scanned_time) < KEY_SCANNED_MS){
        return;
    }

    //如果到了5ms，则进行扫描
    key_last_scanned_time = sys_ms;

    raw = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);

    if(raw == key_last_raw){
        if(key_scanned_count < KEY_SCANNED_COUNT){
            key_scanned_count++;
        }
    }
    else{
        key_scanned_count = 0;
        key_last_raw = raw;
    }
    
    if(key_scanned_count >= KEY_SCANNED_COUNT){
        if(raw != key_stable_state){
            key_stable_state = raw;
            if(key_stable_state == KEY_PRESSED_TASK){
                key_event = KEY_EVENT_PRESSED;
            }
            else{
                key_event = KEY_EVENT_RELEASED;
            }
        }
    }
}

Key_Event Key_GetEvent(void){
    Key_Event event = key_event;
    key_event = KEY_EVENT_NONE;
    return event;
}