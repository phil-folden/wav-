#include <stm32f10x.h> 
#include <stdint.h>
#include "MyDMA.h"
#include "Audio_Timer.h"
#include "Wav.h"


uint8_t dma_buffer[Audio_buf_size];
volatile uint8_t audio_half_request;
volatile uint8_t audio_full_request;
volatile uint8_t underrun = 0;

void MyDMA_Init(void){
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(TIM2->CCR1);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dma_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = Audio_buf_size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize =  DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    DMA_ITConfig(DMA1_Channel3, DMA_IT_HT | DMA_IT_TC | DMA_IT_TE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void MyDMA_Init1(uint8_t entry_index){
    audio_full_request = 0;
    audio_half_request = 0; 
    WAV_Sample(entry_index, dma_buffer);
    WAV_Sample(entry_index, dma_buffer + Audio_buf_half_size);
}

void DMA1_Channel3_IRQHandler(void){
    if(DMA_GetITStatus(DMA1_IT_HT3) != RESET){
        DMA_ClearITPendingBit(DMA1_IT_HT3);
        if(audio_full_request == 1){
            underrun = 1;
        }
        //半传输完成中断处理
        //应该在这里填充dma_buffer的前半部分，即dma_buffer[0]到dma_buffer[Audio_buf_half_size - 1]
        audio_half_request = 1;
    }

    if(DMA_GetITStatus(DMA1_IT_TC3) != RESET){
        DMA_ClearITPendingBit(DMA1_IT_TC3);
        if(audio_half_request == 1){
            underrun = 1;
        }
        //传输完成中断处理
        //应该在这里填充dma_buffer的后半部分，即dma_buffer[Audio_buf_half_size]到dma_buffer[Audio_buf_size - 1]
        audio_full_request = 1;
    }
}