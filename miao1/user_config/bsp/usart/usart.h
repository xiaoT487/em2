
#ifndef MIAO1_USART_H
#define MIAO1_USART_H
#include <usart.h>

#define UART_TYPE_NUM 6
#define BUFFER_SIZE 256
#define COMMAND_MIN_LEN 4

typedef enum
{
    uart1,
    uart2,
    uart3,
    uart5,
    uart7,
    uart10
} uart_type;

void UART_send(uart_type device , uint8_t data , uint16_t length);

void Command_tx_AddReadIndex(uart_type device , uint8_t length);
void Command_rx_AddReadIndex(uart_type device , uint8_t length);
uint8_t Command_tx_Read(uart_type device , uint8_t i);
uint8_t Command_tx_Read_one(uart_type device);
uint8_t Command_rx_Read(uart_type device , uint8_t i);
uint8_t Command_tx_GetLength(uart_type device);
uint8_t Command_rx_GetLength(uart_type device);
uint8_t Command_tx_GetRemain(uart_type device);
uint8_t Command_rx_GetRemain(uart_type device);
uint8_t Command_tx_Write(uart_type device , uint8_t *data, uint8_t length);
uint8_t Command_rx_Write(uart_type device , uint8_t *data, uint8_t length);

#endif //MIAO1_USART_H
