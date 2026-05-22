#ifndef _MYDMA_H
#define _MYDMA_H

#include <stdint.h>

#define Audio_buf_size 4096
#define Audio_buf_half_size (Audio_buf_size / 2)

extern volatile uint8_t audio_half_request;
extern volatile uint8_t audio_full_request;
extern volatile uint8_t underrun;

void MyDMA_Init(void);

void MyDMA_Init1(uint8_t entry_index);

#endif
