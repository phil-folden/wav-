#ifndef _SPI_HARDWARE_H
#define _SPI_HARDWARE_H

#include <stdint.h>

void MySpi_W_cs(uint8_t Bitvalue);

void SPI_Hardware_Init(void);

void SPI_Hardware_Start(void);

void SPI_Hardware_Stop(void);

uint8_t SPI_Hardware_swap(uint8_t data);

void RC522_Write(uint8_t addr, uint8_t data);

#endif