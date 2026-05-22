// #include <stm32f10x.h>    // Device header

// void MySpi_W_cs(uint8_t Bitvalue){
//     GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)Bitvalue);
// }

// void MySpi_W_sck(uint8_t Bitvalue){
//     GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)Bitvalue);
// }

// void MySpi_W_mosi(uint8_t Bitvalue){
//     GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)Bitvalue);
// }

// uint8_t MySpi_R_miso(void){
//     return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
// }

// void SPI_Software_Init(void){
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;  // SCK, MISO, CS
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
//     GPIO_Init(GPIOA, &GPIO_InitStructure);

//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  // MOSI
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_Init(GPIOA, &GPIO_InitStructure);

//     MySpi_W_cs(1);  // 初始状态，CS拉高
//     MySpi_W_sck(0); // 初始状态，SCK拉低
// }

// void SPI_Software_Start(void){
//     MySpi_W_cs(0);  // 拉低CS，开始通信
// }

// void SPI_Software_Stop(void){
//     MySpi_W_cs(1);  // 拉高CS，结束通信
// }

// uint8_t SPI_Software_swap(uint8_t data){
//     uint8_t receivedData = 0x00;

//     for(int i = 0; i < 8; i++){
//         MySpi_W_mosi(data & (0x80 >> i));
//         MySpi_W_sck(1);  // 上升沿，数据有效
//         if(MySpi_R_miso() == 0){
//             receivedData |= (0x00 >> i);
//         }
//         MySpi_W_sck(0);  // 下降沿，准备下一位
//     }
//     return receivedData;
// }