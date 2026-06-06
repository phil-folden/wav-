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
#include "OLED.h"
#include "Scene_Manager.h"

volatile uint8_t sine_index = 0;
extern uint8_t dma_buffer[Audio_buf_size];

volatile uint32_t sys_ms = 0;

void SysTick_Handler(void){
    sys_ms++;
}

int main(void)
{
    Key_Init();
    SysTick_Config(SystemCoreClock / 1000);  // 配置SysTick定时器，每1ms触发一次中断
    Serial_Init();
    OLED_Init();
    Init_WAV();
    Audio_Init();
    Key_Event key_event1 = KEY_EVENT_NONE;
    while(1){
        Audio_Play1();
        Key_Scanned();
        key_event1 = Key_GetEvent();
        Scene_Manager_Flash();
        if(key_event1 == KEY_EVENT_PRESSED){
            if(sceneid == Scene_sonelist){
                Scene_Manager_Handle((Play_Menu_State)0);
            }
            else if(sceneid == Scene_soneplaying){
                Scene_Manager_Handle((Play_Menu_State)play_menu_index);
            }
        }
    }
}
