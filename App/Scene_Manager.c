#include <stm32f10x.h>
#include <stdio.h>
#include "Scene_Manager.h"
#include "Aduio_Player.h"
#include "Rotate.h"
#include "OLED.h"
#include "Wav.h"
#include "Serial.h"

uint8_t need_flash = 0;
int8_t rotate_count = 0;
uint8_t play_menu_index = 0;
uint8_t song_index = 0;
SceneId sceneid = Scene_soneplaying;
static uint8_t song_step = 0, menu_step = 0;

char list_name[5][17]={
    "last",
    "return",
    "next",
    "pause",
    "stop"
};

static void sonelist_Handle(Play_Menu_State state){
    switch(state){
        case Play_Menu_before:
            sceneid = Scene_soneplaying;
            need_flash = 1;
            Audio_Play(song_index, 0);
            Audio_Reseek();
            break;
        case Play_Menu_stopped:

            break;

        case Play_Menu_paused:
            break;

        case Play_Menu_error:
            break;
    }
}

static void soneplaying_Handle(Play_Menu_State state){
    uint32_t offset;
    uint8_t is_playing;
    switch(state){
        case Play_Menu_before:
            if(song_index == 0){
                song_index = sd_count - 1;
            }
            else{
                song_index -= 1;
            }
            Audio_Play(song_index, 0);
            Audio_Reseek();
            need_flash = 1;
            break;

        case Play_Menu_return:
            sceneid = Scene_sonelist;
            Audio_Stop();
            need_flash = 1;
            break;

        case Play_Menu_next:
            if(song_index == sd_count - 1){
                song_index = 0;
            }
            else{
                song_index += 1;
            }
            Audio_Play(song_index, 0);
            Audio_Reseek();
            need_flash = 1;
            break;

        case Play_Menu_paused:
            is_playing = Audio_get_state();
            offset = Audio_get_offset();
            if(is_playing == 1){
                //正在播放，点击就是停止播放
                need_flash = 1;
                Audio_Stop();
            }
            else if(is_playing == 0){
                //没有播放,点击就是继续播放
                Audio_restart();
                Audio_Play(song_index, offset);
                Audio_Reseek();
                need_flash = 1;
            }
            break;

        case Play_Menu_stopped:
                Audio_Stop();
                need_flash = 1;
                break;
            
        case Play_Menu_error:
            Audio_Stop();
            Serial_SendString("Audio error\r\n");
            break;
    }
}

void Scene_Manager_Init(void){
    Rotate_Init();
    sceneid = Scene_sonelist;
    song_index = 0;
    play_menu_index = 0;
    need_flash = 1;
}

void Scene_Manager_Handle(Play_Menu_State state){
    switch(sceneid){
        case Scene_sonelist:
            sonelist_Handle(state);
            break;
        case Scene_soneplaying:
            soneplaying_Handle(state);
            break;
    }
}

static void UI_Sonelist(void){
    uint8_t j[4];
    for(int i = 0; i < 4 && i < sd_count; i++){
        uint8_t temp = song_index + i;
        if(temp >= sd_count){
            temp = temp - sd_count;
        }
        j[i] = temp;
    }

    switch(song_step){
        case(0):
        OLED_ClearTextLine(1);
        OLED_ShowString(1, 1, "->");
        OLED_ShowString(1, 3, (char*)sd_data[j[0]].name);
        need_flash = 1;
        song_step++;
        break;
            
        case(1):
        OLED_ClearTextLine(2);
        OLED_ShowString(2, 3, (char*)sd_data[j[1]].name);
        need_flash = 1;
        song_step++;
        break;

        case(2):
        OLED_ClearTextLine(3);
        OLED_ShowString(3, 3, (char*)sd_data[j[2]].name);
        need_flash = 1;
        song_step++;
        break;

        case(3):
        OLED_ClearTextLine(4);
        OLED_ShowString(4, 3, (char*)sd_data[j[3]].name);
        need_flash = 0;
        song_step = 0;
        break;
    }
}

static void UI_Menulist(void){
    uint8_t j[4];
    for(int i = 0; i < 4 && i < Play_Menu_Count; i++){
        uint8_t temp = play_menu_index + i;
        if(temp >= Play_Menu_Count){
            temp = temp - Play_Menu_Count;
        }
        j[i] = temp;
    }

    switch(menu_step){
        case(0):
        OLED_ClearTextLine(1);
        OLED_ShowString(1, 1, "->");
        OLED_ShowString(1, 3, list_name[j[0]]);
        OLED_ClearTextLine(2);
        OLED_ShowString(2, 3, list_name[j[1]]);
        menu_step++;
        need_flash = 1;
        break;
            
        case(1):
        OLED_ClearTextLine(3);
        OLED_ShowString(3, 3, list_name[j[2]]);
        OLED_ClearTextLine(4);
        OLED_ShowString(4, 3, list_name[j[3]]);
        menu_step = 0;
        need_flash = 0;
        break;

    //     case(2):
    //     OLED_ClearTextLine(3);
    //     OLED_ShowString(3, 3, list_name[j[2]]);
    //     need_flash = 1;
    //     menu_step++;
    //     break;

    //     case(3):
    //     OLED_ClearTextLine(4);
    //     OLED_ShowString(4, 3, list_name[j[3]]);
    //     need_flash = 0;
    //     menu_step = 0;
    //     break;
    }
}

static void OLED_show_Songlist(void){
    uint8_t can_flash = Audio_get_flash();
    if(sd_count == 0){
        return;
    }

    rotate_count = rotate_count % sd_count;
    if(rotate_count < 0){
        song_index = (song_index + sd_count + rotate_count) % sd_count;
    }
    else if(rotate_count > 0){  
        song_index = (song_index + rotate_count) % sd_count;
    }

    if(can_flash == 1){
        UI_Sonelist();
    }
}

static void OLED_show_Soneplaying(void){
    uint8_t can_flash = Audio_get_flash();
    rotate_count = rotate_count % Play_Menu_Count;
    if(rotate_count < 0){
        play_menu_index = (play_menu_index + Play_Menu_Count + rotate_count) % Play_Menu_Count;
    }
    else if(rotate_count > 0){
        play_menu_index = play_menu_index + rotate_count;
        play_menu_index = play_menu_index % Play_Menu_Count;
    }

    uint8_t is_playing = Audio_get_state();
        if(is_playing == 0){
            sprintf(list_name[3], "%s", "play");
        }
        else if(is_playing == 1){
            sprintf(list_name[3], "%s", "pause");
        }

    if(can_flash == 1){
        UI_Menulist();
    }
}

void Scene_Manager_Flash(void){
    if(need_flash == 1){
        rotate_count = get_Rotate_count();
        switch(sceneid){
        case Scene_sonelist:
            OLED_show_Songlist();
            break;
        case Scene_soneplaying:
            OLED_show_Soneplaying();
            break;
        }
    }
}