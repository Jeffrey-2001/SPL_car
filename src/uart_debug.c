#include "uart_debug.h"

/**
 * @brief UART1初始化 (TX=PA9, RX=PA10)
 * @param baud 波特率
 */
void uart_debug_init(uint32_t baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // TX = PA9, 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX = PA10, 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/* printf 重定向到 USART1 */
int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, ptr[i]);
    }
    return len;
}

/* 单字符输出（putchar 也会走这里） */
int __io_putchar(int ch)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, (uint8_t)ch);
    return ch;
}

/**
 * @brief 发送VOFA+ JustFloat协议数据
 * @param data 浮点数组指针
 * @param channel_count 通道数
 * @note  JustFloat格式: 每个通道4字节(float小端序)，尾帧0x00
 *        VOFA+配置: JustFloat引擎，通道数=channel_count
 */
void vofa_justfloat_send(float *data, uint8_t channel_count)
{
    uint8_t *p;
    for (uint8_t i = 0; i < channel_count; i++) {
        p = (uint8_t *)(&data[i]);
        for (uint8_t j = 0; j < 4; j++) {
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
            USART_SendData(USART1, p[j]);
        }
    }
}
