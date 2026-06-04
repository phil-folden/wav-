#ifndef _Audio_Player_
#define _Audio_Player_

extern uint8_t is_end;

void Audio_Stop(void);

void Audio_Start(uint8_t entry_index);

void Audio_Init(void);

uint32_t Audio_Play(uint8_t entry_index, uint32_t offset);

#endif

