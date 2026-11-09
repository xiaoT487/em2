#include <string.h>

#include "usart.h"

uint8_t tx_buffer[UART_TYPE_NUM][BUFFER_SIZE];
uint8_t rx_buffer[UART_TYPE_NUM][BUFFER_SIZE];
uint8_t tx_writeindex[UART_TYPE_NUM];
uint8_t rx_writeindex[UART_TYPE_NUM];
static uint8_t tx_readindex[UART_TYPE_NUM];
uint8_t rx_readindex[UART_TYPE_NUM];

void Command_tx_AddReadIndex(uart_type device , uint8_t length)
{
    tx_readindex[device] += length;
    tx_readindex[device] %= BUFFER_SIZE;
}

void Command_rx_AddReadIndex(uart_type device , uint8_t length)
{
    rx_readindex[device] += length;
    rx_readindex[device] %= BUFFER_SIZE;
}

uint8_t Command_tx_Read(uart_type device , uint8_t i) {
    uint8_t index = i % BUFFER_SIZE;
    return tx_buffer[device][index];
}

uint8_t Command_tx_Read_one(uart_type device) {
    uint8_t index = (tx_readindex[device]++) % BUFFER_SIZE;
    return tx_buffer[device][index];
}

uint8_t Command_rx_Read(uart_type device , uint8_t i) {
    uint8_t index = i % BUFFER_SIZE;
    return rx_buffer[device][index];
}

uint8_t Command_tx_GetLength(uart_type device) {
    return (tx_writeindex[device] + BUFFER_SIZE - tx_readindex[device]) % BUFFER_SIZE;
}

uint8_t Command_rx_GetLength(uart_type device) {
    return (rx_writeindex[device] + BUFFER_SIZE - rx_readindex[device]) % BUFFER_SIZE;
}

uint8_t Command_tx_GetRemain(uart_type device) {
    return BUFFER_SIZE - Command_tx_GetLength(device);
}

uint8_t Command_rx_GetRemain(uart_type device) {
    return BUFFER_SIZE - Command_rx_GetLength(device);
}

uint8_t Command_tx_Write(uart_type device , uint8_t *data, uint8_t length) {

    if (Command_tx_GetRemain(device) < length) {
        return 0;
    }

    if (tx_writeindex[device] + length < BUFFER_SIZE) {
        memcpy(tx_buffer[device] + tx_writeindex[device], data, length);
        tx_writeindex[device] += length;
    } else {
        uint8_t firstLength = BUFFER_SIZE - tx_writeindex[device];
        memcpy(tx_buffer[device] + tx_writeindex[device], data, firstLength);
        memcpy(tx_buffer[device], data + firstLength, length - firstLength);
        tx_writeindex[device] = length - firstLength;
    }
    return length;
}

uint8_t Command_rx_Write(uart_type device , uint8_t *data, uint8_t length) {

    if (Command_rx_GetRemain(device) < length) {
        return 0;
    }

    if (rx_writeindex[device] + length < BUFFER_SIZE) {
        memcpy(rx_buffer[device] + rx_writeindex[device], data, length);
        rx_writeindex[device] += length;
    } else {
        uint8_t firstLength = BUFFER_SIZE - rx_writeindex[device];
        memcpy(rx_buffer[device] + rx_writeindex[device], data, firstLength);
        memcpy(rx_buffer[device], data + firstLength, length - firstLength);
        rx_writeindex[device] = length - firstLength;
    }
    return length;
}