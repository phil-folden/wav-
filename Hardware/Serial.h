#ifndef _SERIAL_H
#define _SERIAL_H


void Serial_Init(void);

void Serial_SendByte(uint8_t data);

void Serial_SendString(const char* str);

void Serial_SendArray(const uint8_t* data);

uint8_t Serial_ReceiveByte(void);

uint8_t Serial_getFlag(void);

uint8_t Serial_getData(void);

void Serial_SendHex(uint8_t data);

void Serial_SendHex32(uint32_t data);

#endif
