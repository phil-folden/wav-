#include <stm32f10x.h>
#include <stdint.h>
#include "MyDMA.h"
#include "Audio_PWM.h"
#include "Audio_PWM.h"
#include "Wav.h"
#include "Aduio_Player.h"

volatile uint8_t audio_file_open = 0;
volatile uint8_t audio_playing = 0;


void Audio_Stop(void){
    audio_playing = 0;
    TIM_DMACmd(TIM3, TIM_DMA_Update, DISABLE);

    TIM_Cmd(TIM3, DISABLE);

    DMA_Cmd(DMA1_Channel3, DISABLE);

    DMA_ClearITPendingBit(DMA1_IT_GL3|
                          DMA1_IT_HT3|
                          DMA1_IT_TC3|
                          DMA1_IT_TE3);

    DMA_ITConfig(DMA1_Channel3,
                DMA_IT_HT | DMA_IT_TC | DMA_IT_TE,
                DISABLE);

    audio_half_request = 0;
    audio_full_request = 0;
    
    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Enable);
    TIM_Cmd(TIM2, ENABLE);

    TIM2->CCR1 = 0x80;
    TIM_GenerateEvent(TIM2, TIM_EventSource_Update);

    

    if(audio_file_open == 1){
        End_FAT();
        audio_file_open = 0;
    }
}

void Audio_Start(uint8_t entry_index){
    audio_playing = 1;
    Audio_Stop();

    Start_FAT(entry_index);

    audio_file_open = 1;

    MyDMA_Init1(entry_index);

    DMA_ITConfig(DMA1_Channel3,
                 DMA_IT_HT | DMA_IT_TC | DMA_IT_TE,
                 ENABLE);

    DMA_Cmd(DMA1_Channel3, ENABLE);

    TIM_DMACmd(TIM3, TIM_DMA_Update, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}