#include <stm32f10x.h> 
#include <stdint.h>
#include <String.h>
#include <stdio.h>
#include "SPI_hardware.h"
#include "Serial.h"
#include "HW226.h"
#include "Wav.h"
#include "MyDMA.h"
#include "Audio_PWM.h"
#include "ff.h"


uint8_t buf1[512];
uint8_t buf2[buf2_maxread];
SD_Data sd_data[WAV_MAX_FILES];
WAV_Info wav_info[WAV_MAX_FILES];

static FATFS fs;
static FIL file;
static FRESULT res;
uint8_t sd_count = 0;
UINT br;


static uint8_t Wav_ToUpper(uint8_t ch){
    if(ch >= 'a' && ch <= 'z'){
        return ch - ('a' - 'A');
    }

    return ch;
}

static uint8_t Wav_IsFileName(const char *name){
    const char *ext = 0;

    while(*name != '\0'){
        if(*name == '.'){
            ext = name + 1;
        }
        name++;
    }

    if(ext == 0){
        return 0;
    }

    return Wav_ToUpper((uint8_t)ext[0]) == 'W'
        && Wav_ToUpper((uint8_t)ext[1]) == 'A'
        && Wav_ToUpper((uint8_t)ext[2]) == 'V'
        && ext[3] == '\0';
}

static void Wav_StoreFileName(const char *filename, SD_Data *data){
    uint8_t name_index = 0;
    uint8_t ext_index = 0;
    uint8_t in_ext = 0;

    data->name[0] = '\0';
    data->extend[0] = '\0';

    while(*filename != '\0'){
        if(*filename == '.'){
            in_ext = 1;
            filename++;
            continue;
        }

        if(in_ext == 0){
            if(name_index < sizeof(data->name) - 1){
                data->name[name_index++] = *filename;
            }
        }
        else{
            if(ext_index < sizeof(data->extend) - 1){
                data->extend[ext_index++] = *filename;
            }
        }

        filename++;
    }

    data->name[name_index] = '\0';
    data->extend[ext_index] = '\0';
}

static void Wav_BuildPath(uint8_t entry_index, char *path){
    strcpy(path, "0:/");
    strcat(path, (char *)sd_data[entry_index].name);

    if(sd_data[entry_index].extend[0] != '\0'){
        strcat(path, ".");
        strcat(path, (char *)sd_data[entry_index].extend);
    }
}

static void Wav_FindFiles(SD_Data *data){
    DIR dir;
    FILINFO fno;

    res = f_mount(&fs, "0:", 1);
    if(res != FR_OK){
        Serial_SendString("f_mount failed: ");
        Serial_SendHex(res);
        Serial_SendString("\r\n");
        return;
    }

    res = f_opendir(&dir, "0:/");
    if(res != FR_OK){
        Serial_SendString("f_opendir failed: ");
        Serial_SendHex(res);
        Serial_SendString("\r\n");
        return;
    }

    while(sd_count < WAV_MAX_FILES){
        res = f_readdir(&dir, &fno);
        if(res != FR_OK){
            Serial_SendString("f_readdir failed: ");
            Serial_SendHex(res);
            Serial_SendString("\r\n");
            break;
        }

        if(fno.fname[0] == '\0'){
            break;
        }

        if((fno.fattrib & AM_DIR) != 0){
            continue;
        }

        if(Wav_IsFileName(fno.fname)){
            Wav_StoreFileName(fno.fname, &data[sd_count]);
            // Serial_SendString("file:");
            // Serial_SendString(fno.fname);
            // Serial_SendString("\r\n");
            sd_count++;
        }
    }

    f_closedir(&dir);

    // Serial_SendString("count:");
    // Serial_SendHex(count);
    // Serial_SendString("\r\n");
}

static void Wav_LoadHeaders(WAV_Info *info){
    char path[WAV_PATH_MAX];
    uint32_t offset;
    uint32_t chunk_size;

    for(int i = 0; i < sd_count; i++){
        Wav_BuildPath(i, path);

        res = f_open(&file, path, FA_READ);
        if(res != FR_OK){
            Serial_SendString("head open failed: ");
            Serial_SendHex(res);
            Serial_SendString("\r\n");
            continue;
        }

        res = f_read(&file, buf1, sizeof(buf1), &br);
        f_close(&file);

        if(res != FR_OK){
            Serial_SendString("head read failed: ");
            Serial_SendHex(res);
            Serial_SendString("\r\n");
            continue;
        }

        if(br < 12){
            Serial_SendString("bad wav head\r\n");
            continue;
        }

        offset = 12;
        while(offset + 8 <= br){
            chunk_size = Read_Little_Endian32(buf1, offset + 4);

            if(buf1[offset] == 'f' && buf1[offset + 1] == 'm' && buf1[offset + 2] == 't' && buf1[offset + 3] == ' '){
                info[i].audio_format = Read_Little_Endian16(buf1, offset + 8);
                info[i].num_channels = Read_Little_Endian16(buf1, offset + 10);
                info[i].sample_rate = Read_Little_Endian32(buf1, offset + 12);
                info[i].byte_rate = Read_Little_Endian32(buf1, offset + 16);
                info[i].block_align = Read_Little_Endian16(buf1, offset + 20);
                info[i].bits_per_sample = Read_Little_Endian16(buf1, offset + 22);
            }
            else if(buf1[offset] == 'd' && buf1[offset + 1] == 'a' && buf1[offset + 2] == 't' && buf1[offset + 3] == 'a'){
                info[i].data_offset = offset + 8;
                info[i].data_size = chunk_size;
                break;
            }

            if(offset + 8 + chunk_size > br){
                break;
            }

            offset += 8 + chunk_size;
            if((chunk_size & 1) != 0){
                offset++;
            }
        }
    }
}

void SD_Data_Init(){
    sd_count = 0;
    for(int i = 0; i < WAV_MAX_FILES; i++){
        sd_data[i].name[0] = '\0';
        sd_data[i].extend[0] = '\0';
        sd_data[i].cluster = 0;
        sd_data[i].file_size = 0;
        wav_info[i].audio_format = 0;
        wav_info[i].num_channels = 0;
        wav_info[i].sample_rate = 0;
        wav_info[i].byte_rate = 0;
        wav_info[i].block_align = 0;
        wav_info[i].bits_per_sample = 0;
        wav_info[i].data_offset = 0;
        wav_info[i].data_size = 0;
    }
}

void FAT32_ReadFile(uint8_t entry_index, SD_Data *data){
    uint32_t sector;
    uint16_t index;
    sector = 8224;
    index = entry_index * 32;

    if(index > 512){
        // OLED_ShowString(1, 1, "Entry index too large");
    }

    uint8_t i = SD_ReadSector(sector, buf1);
    if (i != 0)
    {
        Serial_SendString("\r\n");
        Serial_SendString("Read directory sector failed\r\n");
        // OLED_ShowNum(1, 1, i, 1);
        return;
    }


    for(int i = 0; i < 8; i++){
        data->name[i] = buf1[index + i];
    }
    data->name[8] = '\0';  // 添加字符串结束符  
    for(int i = 0; i < 4; i++){
        data->extend[i] = buf1[index + 8 + i];
    }
    data->extend[3] = '\0';  // 添加字符串结束符
    uint32_t cluster = Read_Little_Endian16(&buf1[index], 20) << 16 | Read_Little_Endian16(&buf1[index], 26);  // 起始簇号
    data->cluster = cluster;
    data->file_size = Read_Little_Endian32(&buf1[index], 28);

    Serial_SendString("\r\n");
    Serial_SendString("File name: ");
    Serial_SendString((char*)data->name);
}

void FAT32_ReadFileData(SD_Data *data){
    uint32_t sector;
    sector = 8224 + (data->cluster - 2) * 16;  // 数据区从�?224个扇区开始，每个�?6个扇�?    
    if (SD_ReadSector(sector, buf1) != 0)
    {
        Serial_SendString("\r\n");
        Serial_SendString("Read file data failed\r\n");
        return;
    }

}

void FAT32_SeekAll(SD_Data *data){
    Wav_FindFiles(data);
    return;
}

void Init_WAV_Head(WAV_Info *wav_info){
    Wav_LoadHeaders(wav_info);
    return;
}

void Init_WAV(){
    SD_Data_Init();

    FAT32_SeekAll(sd_data);
    if(sd_count == 0){
        Serial_SendString("No WAV files\r\n");
        return;
    }

    Init_WAV_Head(wav_info);
}

void Start_FAT(uint8_t entry_index, uint32_t offset){
    char path[17];
    Wav_BuildPath(entry_index, path);
    res = f_open(&file, path, FA_READ);
    if(res != FR_OK){
        Serial_SendString("f_open failed: ");
        Serial_SendHex(res);
        Serial_SendString("\r\n");
        return;
    }

    res = f_lseek(&file, wav_info[entry_index].data_offset + offset);
    if(res != FR_OK){
        Serial_SendString("f_lseek failed: ");
        Serial_SendHex(res);
        Serial_SendString("\r\n");
        f_close(&file);
        return;
    }
}

void End_FAT(void){
    f_close(&file);
}

uint32_t WAV_Sample(uint8_t entry_index, uint8_t* data){
    uint32_t offset = 0;
    uint32_t base;
    uint32_t frame_count;
    int16_t left, right;

    if(wav_info[entry_index].num_channels == 1){
        uint8_t sample8;

        res = f_read(&file, buf2, (uint16_t)Audio_buf_half_size, &br);
        frame_count = br / wav_info[entry_index].block_align;
        for(int i = 0; i < frame_count; i++){
            base = i * wav_info[entry_index].block_align;
            sample8 = buf2[base];
            data[i] = (int16_t)sample8;
        }
    }

    else if(wav_info[entry_index].num_channels == 2){
        uint16_t count = (uint16_t)buf2_maxread / 2;
        count = (uint16_t)Audio_buf_half_size / count;

        for(int i = 0; i < count; i++){
            res = f_read(&file, buf2, (uint16_t)buf2_maxread, &br);
            frame_count = br / wav_info[entry_index].block_align;
            offset += br;
            for(int j = 0; j < frame_count; j++){
                uint8_t left_channel;
                uint8_t right_channel;

                base = j * wav_info[entry_index].block_align;
                left = Read_Little_Endian16(buf2, base);
                right = Read_Little_Endian16(buf2, base + 2);
                left_channel = (uint8_t)((left + 32768) >> 8);
                left_channel = (left_channel * 232) / 255;
                right_channel = (uint8_t)((right + 32768) >> 8);
                right_channel = (right_channel * 232) / 255;
                data[i * 2048 + j * 2] = left_channel;
                data[i * 2048 + j * 2 + 1] = right_channel;
            }
        }
    }

    return offset;
}
