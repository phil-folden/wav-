#ifndef _Audio_PWM_H_
#define _Audio_PWM_H_
#include <stdint.h>

#define AUDIO_TIM1_PSC             ((uint16_t)0)
#define AUDIO_TIM1_ARR             ((uint16_t)232)
#define AUDIO_TIM1_RCR             ((uint8_t)6)

#define AUDIO_PWM_MID_VALUE        ((uint16_t)116)

void Audio_Timer_Init(void);

void Audio_GPIO_Init(void);

uint8_t PCM16_To_PWM(int16_t sample);

void Audio_PWM_SetSample(uint16_t sample);

#endif
