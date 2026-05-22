#include <stm32f10x.h> 
#include "SPI_hardware.h"
#include "Serial.h"
#include "Wav.h"

#include <stdint.h>

//先扫描根目录8224到8239。每32个字节是一个目录项
// 目录项：
// +0 ~ +7     文件名，8 字节
// +8 ~ +10    扩展名，3 字节
// +11         属性
// +20 ~ +21   起始簇高 16 位
// +26 ~ +27   起始簇低 16 位
// +28 ~ +31   文件大小
//起始簇高和起始簇低拼起来得到信息的起始簇号，然后可以知道文件的数据的扇区，在这里一个簇是16个扇区

void Delay_ms(uint16_t ms)
{
    uint16_t i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 7200; j++);
}

//发送Dummy Clock，SD卡需要至少74个时钟周期来完成上电初始化
void SD_SendDummyClock(void){
    SPI_Hardware_Stop();  // 开始通信
    for(int i = 0; i < 10; i++){
        SPI_Hardware_swap(0xFF);  // 发送0xFF作为Dummy Clock
    }
}

uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg, uint8_t crc){

    SPI_Hardware_Start();  // 开始通信

    SPI_Hardware_swap(0xFF);  // 发送一个Dummy Byte

    SPI_Hardware_swap(0x40 | cmd);  // 发送命令，最高位置1
    //arg >> 24:代表着2进制向右移24位，就是16进制向右移动6位
    SPI_Hardware_swap((arg >> 24) & 0xFF);  // 发送参数的高字节
    //比如0x12345678 >> 24 & 0xFF 就是0x00000012 & 0x000000FF, 就是0x12 & 0xFF 
    SPI_Hardware_swap((arg >> 16) & 0xFF);  // 发送参数的中字节
    SPI_Hardware_swap((arg >> 8) & 0xFF);   // 发送参数的低字节
    SPI_Hardware_swap(arg & 0xFF);          // 发送参数的最低字节
    SPI_Hardware_swap(crc);  // 发送CRC

    //等待响应，SD卡会在8个时钟周期内返回响应
    uint8_t response;
    for(int i = 0; i < 8; i++){
        response = SPI_Hardware_swap(0xFF);  // 发送Dummy Byte并读取响应
        if(response != 0xFF){  // 如果响应不是0xFF，说明SD卡已经响应
            return response;
        }
    }
    return 0xFF;  // 如果没有响应，返回0xFF
}

uint8_t SD_WaitReady(void){
    uint8_t response;
    for(int i = 0; i < 500; i++){  // 等待最多500ms
        response = SPI_Hardware_swap(0xFF);  // 发送Dummy Byte并读取响应
        if(response == 0xFF){  // 如果响应是0xFF，说明SD卡准备好了
            return 1;
        }
        Delay_ms(1);  // 等待1ms后再次检查
    }
    return 0;  // 超时未准备好
}

uint8_t SD_Init(void){
    uint8_t response;
    uint32_t retry;

    SPI_Hardware_Init();  // 初始化SPI接口
    SD_SendDummyClock();  // 发送Dummy Clock

    //发送CMD0，进入Idle状态
    response = SD_SendCmd(0, 0, 0x95);  // CMD0的CRC必须是0x95

    SPI_Hardware_Stop();  // 拉高CS
    SPI_Hardware_swap(0xFF);  // 发送一个Dummy Byte

    if(response != 0x01){  // CMD0成功后会返回0x01
        return 1;  // CMD0失败，SD卡未进入Idle状态
    }

    response = SD_SendCmd(1, 0, 0xFF);  // 发送CMD1，初始化SD卡
    SPI_Hardware_Stop();  // 拉高CS
    SPI_Hardware_swap(0xFF);  // 发送一个Dummy Byte

    retry = 5000;

    do{
        response = SD_SendCmd(1, 0, 0xFF);  // 发送CMD1，等待SD卡完成初始化
        SPI_Hardware_Stop();  // 拉高CS
        SPI_Hardware_swap(0xFF);  // 发送一个Dummy Byte
        if(retry -- == 0){
            return 2;  // 初始化超时
        }
        
    }while(response != 0x00);  // CMD1成功后会返回0x00

    response = SD_SendCmd(16, 512, 0xFF);  // 设置块长度为512字节
    SPI_Hardware_Stop();  // 拉高CS
    SPI_Hardware_swap(0xFF);  // 发送一个Dummy Byte

    if(response != 0x00){  // CMD16成功后会返回0x00
        return 3;  // 设置块长度失败
    }

    return 0;  // SD卡初始化成功
}

uint8_t SD_ReadBlock(uint32_t addr, uint8_t *buf){
    uint8_t r1;
    uint8_t token;
    uint32_t retry;

    r1 = SD_SendCmd(17, addr, 0xFF);  // 发送CMD17，读取单块数据

    if(r1 != 0x00){  // CMD17成功后会返回0x00
        return 1;  // 发送CMD17失败
    }

    retry = 5000;
    do{
        token = SPI_Hardware_swap(0xFF);  // 等待数据令牌
        if(retry -- == 0){
            return 2;  // 等待数据令牌超时
        }
    }while(token == 0xFF);  // 数据令牌不是0xFF时，说明数据已经准备好

     if (token != 0xFE)
    {
        SPI_Hardware_Stop();
        SPI_Hardware_swap(0xFF);
        return 3;
    }
    
    for(int i = 0; i < 512; i++){
        buf[i] = SPI_Hardware_swap(0xFF);  // 读取数据块
    }

    SPI_Hardware_swap(0xFF);  // 读取CRC高字节（可以忽略）
    SPI_Hardware_swap(0xFF);  // 读取CRC低字节（可以忽略）

    SPI_Hardware_Stop();  // 拉高CS
    SPI_Hardware_swap(0xFF);  // 发送一个Dummy Byte

    return 0;  // 读取数据块成功
}

void SD_Init_Check(uint32_t block){
     int result;
    Serial_SendString("\r\n");

    result = SD_Init();
    if(result !=0 ){
        Serial_SendByte(result);
        while(1);
    }
    uint8_t buf3[512];
    result = SD_ReadBlock(block * 512, buf3);

    if(result == 1){
        Serial_SendString("CMD17F");
        while(1);
    }
    else if(result == 2){
        Serial_SendString("Token Timeout");
        while(1);
    }
}

uint8_t SD_ReadSector(uint32_t block, uint8_t *buf){
    return SD_ReadBlock(block * 512, buf);
}

uint16_t Read_Big_Endian16(uint8_t *buf, uint16_t offset){
    return (uint16_t)buf[offset] << 8 | (uint16_t)buf[offset + 1];
}

uint32_t Read_Big_Endian32(uint8_t *buf, uint16_t offset){
    return (uint32_t)buf[offset] << 24 | (uint32_t)buf[offset + 1] << 16 | (uint32_t)buf[offset + 2] << 8 | (uint32_t)buf[offset + 3];
}

uint16_t Read_Little_Endian16(uint8_t *buf, uint16_t offset){
    return (uint16_t)buf[offset] | (uint16_t)buf[offset + 1] << 8;
}

uint32_t Read_Little_Endian32(uint8_t *buf, uint16_t offset){
    return (uint32_t)buf[offset] | (uint32_t)buf[offset + 1] << 8 | (uint32_t)buf[offset + 2] << 16 | (uint32_t)buf[offset + 3] << 24;
}

void SD_Test(uint32_t block){
    int result;
    Serial_SendString("\r\n");

    result = SD_Init();
    if(result !=0 ){
        Serial_SendByte(result);
        while(1);
    }
    uint8_t buf1[512];
    result = SD_ReadBlock(block * 512, buf1);

    if(result == 1){
        Serial_SendString("CMD17F");
        while(1);
    }
    else if(result == 2){
        Serial_SendString("Token Timeout");
        while(1);
    }
    

    // Serial_SendString("Block ");
    // Serial_SendHex(block);
    // Serial_SendString(": ");

    // for(int i = 0; i < 16; i++){
    // Serial_SendHex(buf1[i]);
    // Serial_SendByte(' ');
    // }

    //这里读取MBR的分区表，分区表位于MBR的第446字节开始，每个分区表项占16字节，一共4个分区表项
    //MBR存储着第一个分区的位置和大小，第 1 个分区表项：buf1[446] ~ buf1[461]
    //第 2 个分区表项：buf1[462] ~ buf1[477]
    //第 3 个分区表项：buf1[478] ~ buf1[493]
    //第 4 个分区表项：buf1[494] ~ buf1[509]
    //一个分区的表项的后8个字节先是4个字节的位置，再是4个字节的大小，是小端的表现方式


    // uint32_t part_start;

    // part_start =  (uint32_t)buf1[454]
    //         | ((uint32_t)buf1[455] << 8)
    //         | ((uint32_t)buf1[456] << 16)
    //         | ((uint32_t)buf1[457] << 24);

    // Serial_SendString("Partition start sector: ");
    // Serial_SendHex32(part_start);
    // Serial_SendString("\r\n");

    //uint32_t part_size;
    // part_size =  (uint32_t)buf1[458]
    //         | ((uint32_t)buf1[459] << 8)
    //         | ((uint32_t)buf1[460] << 16)
    //         | ((uint32_t)buf1[461] << 24);

    // Serial_SendString("Partition size (sectors): ");
    // Serial_SendHex32(part_size);
    // Serial_SendString("\r\n");

    //第一个分区从第32个扇区开始,在第一个分区的第一个扇区里存储的就是FAT32的根目录，这里面存储着每个扇区多少字节
    // 每个簇有多少个扇区
    // FAT 表从哪里开始
    // 数据区从哪里开始
    // 根目录从哪里开始

    // bytes_per_sector       = 512
    // sectors_per_cluster    = 16
    // reserved_sector_count  = 6314
    // num_fats               = 2
    // fat_size               = 939
    // root_cluster           = 2

    // SD_ReadBlock(part_start * 512, buf1);
    // Serial_SendString("FAT boot sector first 16: ");
    // Serial_SendString("Boot sector first 64:\r\n");

    //根据读出来的信息:保留区共6314个扇区，两个FAT表各939个扇区，数据区从8224扇区开始
    //8224扇区一开始存储的是
//    SD_ReadSector(block, buf1);
//     for(int i = 0; i < 512; i++){
//         Serial_SendHex(buf1[i]);
//         Serial_SendByte(' ');
//         if((i + 1) % 16 == 0){
//         Serial_SendString("\r\n");
//         }
//     }


    // FAT32_ReadFile(10, &sd_data[0]);
    // FAT32_ReadFileData(&sd_data[0]);

    //  for(int i = 0; i < 128; i++){
    //     Serial_SendHex(buf1[i]);
    //     Serial_SendByte(' ');
    //     if((i + 1) % 16 == 0){
    //     Serial_SendString("\r\n");
    //     }
    //  }
}

