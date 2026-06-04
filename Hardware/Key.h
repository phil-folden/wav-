#ifndef _KEY_H
#define _KEY_H

extern uint8_t key_count;

typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESSED,
    KEY_EVENT_RELEASED,
}Key_Event;

void Key_Init(void);

void Key_Scanned(void);

Key_Event Key_GetEvent(void);



#endif
