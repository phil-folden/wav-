#include <stm32f10x.h>
#include <stdio.h>
#include "Scene_Manager.h"
#include "Aduio_Player.h"
#include "Rotate.h"
#include "OLED.h"
#include "Wav.h"
#include "Serial.h"

uint8_t need_flash = 0;
uint8_t is_playing = 0;
int8_t rotate_count = 0;
uint8_t restart = 0;
uint8_t list_index = 0;
uint8_t song_index = 0;
uint32_t scene_offset = 0;
SceneId sceneid = Scene_soneplaying;

char list_name[5][17]={
    "last",
    "return",
    "next",
    "pause",
    "stop"
};

static void sonelist_Handle(Audio_State state){
    switch(state){
        case Audio_playing_or_before:
            sceneid = Scene_soneplaying;
            need_flash = 1;
            is_playing = 1;
            scene_offset = Audio_Play(song_index, 0);
            break;
        case Audio_stopped:

            break;

        case Audio_paused:
            break;

        case Audio_error:
            break;
    }
}

static void soneplaying_Handle(Audio_State state){
    switch(state){
        case Audio_playing_or_before:
            if(song_index == 0){
                song_index = sd_count - 1;
            }
            else{
                song_index -= 1;
            }
            is_end = 1;
            Audio_Stop();
            scene_offset = 0;
            restart = 1;
            is_playing = 1;
            need_flash = 1;
            break;

        case Audio_return:
            sceneid = Scene_sonelist;
            scene_offset = 0;
            Audio_Stop();
            is_playing = 0;
            is_end = 1;
            need_flash = 1;
            break;

        case Audio_next:
            if(song_index == sd_count - 1){
                song_index = 0;
            }
            else{
                song_index += 1;
            }
            is_end = 1;
            Audio_Stop();
            scene_offset = 0;
            restart = 1;
            is_playing = 1;
            need_flash = 1;
            break;

        case Audio_paused:
            if(is_playing == 1){
                //正在播放，点击就是停止播放
                need_flash = 1;
                is_playing = 0;
                is_end = 1;
                Audio_Stop();
            }
            else if(is_playing == 0){
                //没有播放,点击就是继续播放
                need_flash = 1;
                is_playing = 1;
                is_end = 1;
                restart = 1;
            }
            break;

        case Audio_stopped:
                is_end = 1;
                Audio_Stop();
                scene_offset = 0;
                is_playing = 0;
                need_flash = 1;
                break;
            
        case Audio_error:
            is_end = 0;
            Serial_SendString("Audio error\r\n");
            break;
    }
}

void Scene_Manager_Init(void){
    Rotate_Init();
    sceneid = Scene_sonelist;
    song_index = 0;
    list_index = 0;
    need_flash = 1;
    is_playing = 1;
}

void Scene_Manager_Handle(Audio_State state){
    switch(sceneid){
        case Scene_sonelist:
            sonelist_Handle(state);
            break;
        case Scene_soneplaying:
            soneplaying_Handle(state);
            break;
    }
}

static void OLED_show_Songlist(void){
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

    OLED_Clear();
    for(int i = 0; i < 4 && i < sd_count; i++){
        uint8_t j = song_index + i;
        if(j >= sd_count){
            j = j - sd_count;
        }

        if(i == 0){
            OLED_ShowString(1, 1, "->");
            OLED_ShowString(i + 1, 3, (char*)sd_data[j].name);
        }
        else{
            OLED_ShowString(i + 1, 1, (char*)sd_data[j].name);
        }
    }
}

static void OLED_show_Soneplaying(void){
    OLED_Clear();
    rotate_count = rotate_count % 5;
    if(rotate_count < 0){
        list_index = (list_index + 5 + rotate_count) % 5;
    }
    else if(rotate_count > 0){
        list_index = list_index + rotate_count;
        list_index = list_index % 5;
    }

    if(is_playing == 0){
        //??????
        sprintf(list_name[3], "%s", "play");
    }
    else if(is_playing == 1){
        //??????
        sprintf(list_name[3], "%s", "pause");
    }

    for(int i = 0; i < 4; i++){
        uint8_t j = list_index + i;
        if(j >= 5){
            j = j - 5;
        }
        if(i == 0){
            OLED_ShowString(1, 1, "->");
            OLED_ShowString(i + 1, 3, list_name[j]);
        }
        else{
            OLED_ShowString(i + 1, 1, list_name[j]);
        }
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
        need_flash = 0;
    }
}

