#ifndef _Audio_Player_
#define _Audio_Player_

typedef enum{
    Audio_State_Stop = 0,
    Audio_State_Playing,
    Audio_State_Paused,
    Audio_State_Finished,
    Audio_State_Error
} Audio_State;

typedef struct{
    Audio_State state;
    uint32_t offset;
    uint8_t index; 
    uint8_t restart; //记录音乐是否重新打开，如果是就要加上offset，且要打开文件
    uint8_t file_open; //用于记录文件有没有打开，避免End_FAT重复打开或关闭文件
    uint8_t lsleek; //用于Scene_Manager里点击last,next那些需要重新定位的标志
}Audio_t;

void Audio_Stop(void);

void Audio_Start(void);

void Audio_Play(uint8_t entry_index, uint32_t offset);

void Audio_Stop1(void);

void Audio_Start1(uint8_t entry_index);

void Audio_Play1(void);

void Audio_Init(void);

uint32_t Audio_get_offset(void);

uint8_t Audio_get_flash(void);

uint8_t Audio_get_state(void);

void Audio_restart(void);

void Audio_Reseek(void);

#endif

