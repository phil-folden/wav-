#include <stm32f10x.h>
#include <stdint.h>
#include "MyDMA.h"
#include "Audio_PWM.h"
#include "Audio_PWM.h"
#include "Wav.h"
#include "Aduio_Player.h"
#include "Key.h"
#include "Scene_Manager.h"
#include "Serial.h"

volatile uint8_t audio_file_open = 0;
volatile uint8_t audio_playing = 0;
extern uint8_t dma_buffer[Audio_buf_size];
uint32_t offset1 = 0;
uint8_t is_end = 0;
Key_Event key_event = KEY_EVENT_NONE;

void Audio_Stop(void){
    audio_playing = 0;
    TIM_DMACmd(TIM1, TIM_DMA_Update, DISABLE);

    TIM_Cmd(TIM1, DISABLE);

    DMA_Cmd(DMA1_Channel5, DISABLE);

    DMA_ClearITPendingBit(DMA1_IT_GL5|
                          DMA1_IT_HT5|
                          DMA1_IT_TC5|
                          DMA1_IT_TE5);

    DMA_ITConfig(DMA1_Channel5,
                DMA_IT_HT | DMA_IT_TC | DMA_IT_TE,
                DISABLE);

    audio_half_request = 0;
    audio_full_request = 0;

    TIM1->CCR1 = AUDIO_PWM_MID_VALUE;
    TIM1->CCR2 = AUDIO_PWM_MID_VALUE;

    TIM_SetCounter(TIM1, 0);
    TIM_GenerateEvent(TIM1, TIM_EventSource_Update);
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    TIM_Cmd(TIM1, ENABLE);

    if(audio_file_open == 1){
        End_FAT();
        audio_file_open = 0;
    }
}

void Audio_Start(uint8_t entry_index){
    Audio_Stop();
    audio_playing = 1;

    audio_file_open = 1;

    MyDMA_Init1(entry_index);

    DMA_ITConfig(DMA1_Channel5,
                 DMA_IT_HT | DMA_IT_TC | DMA_IT_TE,
                 ENABLE);

    DMA_Cmd(DMA1_Channel5, ENABLE);

    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
}

void Audio_Init(void){
    Init_WAV();
    Audio_GPIO_Init();
    Audio_Timer_Init();
    MyDMA_Init();
    Scene_Manager_Init();
}

uint32_t Audio_Play(uint8_t entry_index, uint32_t offset){
    offset1 = offset;
    is_end = 0;

    Start_FAT(entry_index, offset);
    Audio_Start(entry_index);

    while (is_end == 0)
    {   
        if(audio_half_request == 1){
            audio_half_request = 0;
            offset1 += WAV_Sample(entry_index, dma_buffer);
        }

        if(audio_full_request == 1){
            audio_full_request = 0;
            offset1 += WAV_Sample(entry_index, dma_buffer + Audio_buf_half_size);
        }
        Scene_Manager_Flash();
        Key_Scanned();
        key_event = Key_GetEvent();
        if(key_event == KEY_EVENT_PRESSED){
            if(sceneid == Scene_sonelist){
                Scene_Manager_Handle((Audio_State)0);
            }
            else if(sceneid == Scene_soneplaying){
                Scene_Manager_Handle((Audio_State)list_index);
            }
        }
    }
    if(restart == 1 || sceneid == Scene_sonelist || list_index == Audio_stopped){
        offset1 = scene_offset;
    }
    Audio_Stop();
    is_end = 0;
    return offset1;
}
