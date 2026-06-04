#ifndef _HW226_H
#define _HW226_H

#include <stdint.h>

void SD_SendDummyClock(void);

uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg, uint8_t crc);

uint8_t SD_WaitReady(void);

uint8_t SD_Init(void);

void SD_Init_Check(uint32_t block);

uint8_t SD_ReadBlock(uint32_t addr, uint8_t *buf);

uint8_t SD_ReadSector(uint32_t block, uint8_t *buf);

void SD_Test(uint32_t block);

uint16_t Read_Big_Endian16(uint8_t *buf, uint16_t offset);

uint32_t Read_Big_Endian32(uint8_t *buf, uint16_t offset);

uint16_t Read_Little_Endian16(uint8_t *buf, uint16_t offset);

uint32_t Read_Little_Endian32(uint8_t *buf, uint16_t offset);


#endif
