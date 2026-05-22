#include <stm32f10x.h>    // Device header

uint8_t Serial_data;
uint8_t Serial_flag;

void Serial_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  // TX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  // RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  // 启用发送和接收
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // 使能接收中断
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  // USART1_IRQn
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void Serial_SendByte(uint8_t data){
    
    USART_SendData(USART1, data);
    
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

}

void Serial_SendHex(uint8_t data){
    uint8_t high = data >> 4;  // 获取高4位
    uint8_t low = data & 0x0F;  // 获取低4位

    if(high < 10){
        Serial_SendByte('0' + high);
    }
    else{
        Serial_SendByte(high - 10  + 'A');
    }

    if(low < 10){
        Serial_SendByte('0' + low);
    }
    else{
        Serial_SendByte(low - 10  + 'A');
    }
}

void Serial_SendHex32(uint32_t data){
    Serial_SendHex(data >> 24 & 0xFF);
    Serial_SendHex(data >> 16 & 0xFF);
    Serial_SendHex(data >> 8 & 0xFF);
    Serial_SendHex(data & 0xFF);
}

void Serial_SendString(const char* str){
    while(*str){
        Serial_SendByte(*str++);
    }
}

void Serial_SendArray(const uint8_t* data){
    while(*data){
        Serial_SendByte(*data++);
    }
}

uint8_t Serial_getFlag(void){
    if(Serial_flag == 1){
        Serial_flag = 0;
        return 1;
    }
    return 0;
}

uint8_t Serial_getData(void){
    return Serial_data;
}

void USART1_IRQHandler(void){
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET){
        Serial_data = USART_ReceiveData(USART1);
        Serial_flag = 1;  // 设置标志位，表示接收到数据
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // 清除中断标志位
    }
}
