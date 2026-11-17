
#ifndef MIAO1_USART_H
#define MIAO1_USART_H
#include <usart.h>

#define BUFFER_SIZE 256
#define RX_DMA_SIZE 128

typedef enum
{
    uart1,
    uart2,
    uart3,
    uart5,
    uart7, 
    uart10,
    UART_TYPE_NUM
} uart_type;

void uart_init(void);
void UART_send(uart_type device , uint8_t *data , uint16_t length);
void UART_send_async(uart_type device , uint8_t *data , uint16_t length);
void UART_receive(uart_type device);

void Command_tx_AddReadIndex(uart_type device , uint8_t length);
void Command_rx_AddReadIndex(uart_type device , uint8_t length);
uint8_t Command_tx_Read(uart_type device , uint8_t i);
uint8_t Command_rx_Read(uart_type device , uint8_t i);
uint8_t Command_tx_GetLength(uart_type device);
uint8_t Command_rx_GetLength(uart_type device);
uint8_t Command_tx_GetRemain(uart_type device);
uint8_t Command_rx_GetRemain(uart_type device);
uint8_t Command_tx_GetContinuous(uart_type device, uint8_t **ptr);
uint8_t Command_tx_Write(uart_type device , uint8_t *data, uint8_t length);
uint8_t Command_rx_Write(uart_type device , uint8_t *data, uint8_t length);
uint8_t Command_rx_Peek(uart_type device, uint8_t offset);
uint8_t Command_rx_ReadData(uart_type device, uint8_t *data, uint8_t length);

#endif //MIAO1_USART_H
