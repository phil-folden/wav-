#include "stm32f10x.h"
#include <Serial.h>
#include <SPI_hardware.h>
#include <HW226.h>
#include <Wav.h>
#include <Audio_PWM.h>
#include <MyDMA.h>
#include <Audio_Timer.h>
#include "Key.h"
#include "Aduio_Player.h"

volatile uint8_t sine_index = 0;
extern uint8_t dma_buffer[Audio_buf_size];


int main(void)
{
    Serial_Init();

    Key_Init();

    Init_WAV();

    Init_Audio_PWM();
    MyDMA_Init();
    Init_Audio_Timer();;

    Audio_Start(0);

    uint8_t i = 0;
    while (key_count != 1)
    {
        if(underrun == 1){
            underrun = 0;
            Serial_SendString("underrun");
            Serial_SendByte(i);
            i++;
        }
        if(i == 0){
            Serial_SendString("underrun");
            i++;
        }
        if(audio_half_request == 1){
            audio_half_request = 0;
            WAV_Sample(0, dma_buffer);
        }

        if(audio_full_request == 1){
            audio_full_request = 0;
            WAV_Sample(0, dma_buffer + Audio_buf_half_size);
        }
    }
    Serial_SendByte(TIM2->CCR1);
    Audio_Stop();
    Serial_SendByte(TIM2->CCR1);
}

