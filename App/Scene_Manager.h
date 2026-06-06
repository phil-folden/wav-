#ifndef _SCENE_MANAGER_H
#define _SCENE_MANAGER_H

#include <stdint.h>

#define Play_Menu_Count 5

extern uint8_t need_flash;
extern uint8_t play_menu_index;
extern uint8_t song_index;

typedef enum{
    Scene_sonelist = 0,
    Scene_soneplaying
} SceneId;

typedef enum{
    Play_Menu_before = 0,
    Play_Menu_return,
    Play_Menu_next,
    Play_Menu_paused,
    Play_Menu_stopped,
    Play_Menu_error
} Play_Menu_State;

extern SceneId sceneid;
extern Play_Menu_State state;

void Scene_Manager_Init(void);
void Scene_Manager_Handle(Play_Menu_State state);
void Scene_Manager_Flash(void);


#endif
