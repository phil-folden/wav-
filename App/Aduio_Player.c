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

extern uint8_t dma_buffer[Audio_buf_size];
static Audio_t audio;
static uint8_t OLED_CAN_FLASH;
uint32_t offset1 = 0;
Key_Event key_event = KEY_EVENT_NONE;

void Audio_Stop(void){
    audio.state = Audio_State_Stop;
    audio.file_open = 0;
}

void Audio_Start(){
    audio.state = Audio_State_Playing;
    audio.file_open = 1;
}

void Audio_Play(uint8_t entry_index, uint32_t offset){
    audio.state = Audio_State_Playing;
    audio.offset = offset;
    audio.index = entry_index;
}

void Audio_Stop1(void){
    audio.state = Audio_State_Stop;
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

    if(audio.file_open == 1){
        End_FAT();
        audio.file_open = 0;
    }
}

void Audio_Start1(uint8_t entry_index){
    Audio_Stop();
    audio.state = Audio_State_Playing;

    MyDMA_Init1(entry_index);

    DMA_ITConfig(DMA1_Channel5,
                 DMA_IT_HT | DMA_IT_TC | DMA_IT_TE,
                 ENABLE);

    DMA_Cmd(DMA1_Channel5, ENABLE);

    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
}

void Audio_Play1(void){
    uint32_t offset = 0;
    switch(audio.state){
        case(Audio_State_Playing):

            if(audio.lsleek == 1){
                //需要重新定位
                Start_FAT(audio.index, audio.offset);
                audio.lsleek = 0;
            }

            if(audio.restart == 1){
                //
                Audio_Start1(audio.index);
                audio.restart = 0;
            }

            if(audio_half_request == 1){
                audio_half_request = 0;
                offset += WAV_Sample(audio.index, dma_buffer);
            }

            if(audio_full_request == 1){
                audio_full_request = 0;
                offset += WAV_Sample(audio.index, dma_buffer + Audio_buf_half_size);
            }
            audio.offset += offset;
            if(audio_half_request == 0 && audio_full_request == 0){
                OLED_CAN_FLASH = 1;
            }
            if(audio_half_request == 1 || audio_full_request == 1){
                OLED_CAN_FLASH = 0;
            }
            break;

        case(Audio_State_Stop):
            Audio_Stop1();
            break;
            
        case(Audio_State_Paused):
            audio.restart = 0;
            Audio_Stop1();
            break;
        
        case(Audio_State_Finished):
            break;

        case(Audio_State_Error):
            break;
            
    }
}

void Audio_Init(void){
    Init_WAV();
    Audio_GPIO_Init();
    Audio_Timer_Init();
    MyDMA_Init();
    Scene_Manager_Init();
    OLED_CAN_FLASH = 1;
    audio.restart = 1;
}

// uint32_t Audio_Play(uint8_t entry_index, uint32_t offset){
//     offset1 = offset;
//     is_end = 0;

//     Start_FAT(entry_index, offset);
//     Audio_Start(entry_index);

//     while (is_end == 0)
//     {   
//         if(audio_half_request == 1){
//             audio_half_request = 0;
//             offset1 += WAV_Sample(entry_index, dma_buffer);
//         }

//         if(audio_full_request == 1){
//             audio_full_request = 0;
//             offset1 += WAV_Sample(entry_index, dma_buffer + Audio_buf_half_size);
//         }
//         Scene_Manager_Flash();
//         Key_Scanned();
//         key_event = Key_GetEvent();
//         if(key_event == KEY_EVENT_PRESSED){
//             if(sceneid == Scene_sonelist){
//                 Scene_Manager_Handle((Play_Menu_State)0);
//             }
//             else if(sceneid == Scene_soneplaying){
//                 Scene_Manager_Handle((Play_Menu_State)play_menu_index);
//             }
//         }
//     }
//     if(restart == 1 || sceneid == Scene_sonelist || play_menu_index == Play_Menu_stopped){
//         offset1 = scene_offset;
//     }
//     Audio_Stop();
//     is_end = 0;
//     return offset1;
// }

uint32_t Audio_get_offset(){
    return audio.offset;
}

uint8_t Audio_get_flash(){
    return OLED_CAN_FLASH;
}

uint8_t Audio_get_state(){
    if(audio.state == Audio_State_Playing){
        return 1;
    }
    return 0;
}

void Audio_restart(){
    audio.restart = 1;
}

void Audio_Reseek(){
    audio.lsleek = 1;
}

