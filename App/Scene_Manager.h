#ifndef _SCENE_MANAGER_H
#define _SCENE_MANAGER_H

#include <stdint.h>

extern uint8_t need_flash;
extern uint8_t list_index;
extern uint8_t song_index;
extern uint8_t restart;
extern uint32_t scene_offset;

typedef enum{
    Scene_sonelist = 0,
    Scene_soneplaying
} SceneId;

typedef enum{
    Audio_playing_or_before = 0,
    Audio_return,
    Audio_next,
    Audio_paused,
    Audio_stopped,
    Audio_error
} Audio_State;

extern SceneId sceneid;
extern Audio_State state;

void Scene_Manager_Init(void);
void Scene_Manager_Handle(Audio_State state);
void Scene_Manager_Flash(void);


#endif
