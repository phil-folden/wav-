#ifndef _WAV_H_
#define _WAV_H_

#include <stdint.h>

#define WAV_MAX_FILES 16
#define WAV_PATH_MAX 17
#define ADUIO_VOLUMN 100
#define buf2_maxread 4096

extern uint8_t buf1[512];
extern uint8_t buf2[buf2_maxread];
extern uint8_t sd_count;

typedef struct {
    uint8_t name[9];       // 文件名
    uint8_t extend[4];     // 扩展名
    uint32_t cluster;    // 起始簇号
    uint32_t file_size;  // 文件大小
} SD_Data;

extern SD_Data sd_data[WAV_MAX_FILES];

typedef struct{
    uint16_t audio_format;      // 音频格式，1表示PCM
    uint16_t num_channels;      // 声道数，1表示单声道，2表示立体声
    uint32_t sample_rate;       // 采样率，例如44100
    uint32_t byte_rate;         // 字节率，等于采样率 * 声道数 * 每样本字节数
    uint32_t block_align;       // 块对齐，等于声道数 * 每样本字节数
    uint16_t bits_per_sample;   // 每样本位数，例如16
    uint32_t data_offset;       // 数据区偏移量，表示从文件开始到数据区的字节数
    uint32_t data_size;         // 数据区大小，表示数据区的字节数
}WAV_Info;

extern WAV_Info wav_info[WAV_MAX_FILES];

void FAT32_ReadFile(uint8_t entry_index, SD_Data *data);

void FAT32_ReadFileData(SD_Data *data);

void FAT32_SeekAll(SD_Data *data);

void Init_WAV_Head(WAV_Info *wav_info);

void Init_WAV(void);

uint32_t WAV_Sample(uint8_t entry_index, uint8_t* data);

void Start_FAT(uint8_t entry_index, uint32_t offset);

void End_FAT(void);

#endif
