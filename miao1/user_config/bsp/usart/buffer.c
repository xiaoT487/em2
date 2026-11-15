#include <string.h>

#include "uart.h"

__attribute__((section(".RAM_D1"), aligned(32)))
static uint8_t tx_buffer[UART_TYPE_NUM][BUFFER_SIZE];
static uint8_t rx_buffer[UART_TYPE_NUM][BUFFER_SIZE];
volatile static uint16_t tx_writeindex[UART_TYPE_NUM];
volatile static uint16_t rx_writeindex[UART_TYPE_NUM];
volatile static uint16_t tx_readindex[UART_TYPE_NUM];
volatile static uint16_t rx_readindex[UART_TYPE_NUM];

volatile static uint16_t tx_overflow_cnt[UART_TYPE_NUM];
volatile static uint16_t rx_overflow_cnt[UART_TYPE_NUM];

#define DEVICE_VALID(dev) ((dev) < UART_TYPE_NUM)

void Command_tx_AddReadIndex(uart_type device , uint16_t length)
{
    if (!DEVICE_VALID(device)) return;
    tx_readindex[device] += length;
    tx_readindex[device] %= BUFFER_SIZE;
}

void Command_rx_AddReadIndex(uart_type device , uint16_t length)
{
    if (!DEVICE_VALID(device)) return;
    rx_readindex[device] += length;
    rx_readindex[device] %= BUFFER_SIZE;
}

int Command_tx_Read(uart_type device , uint16_t i) {
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    uint8_t index = i % BUFFER_SIZE;
    return tx_buffer[device][index];
}

int Command_rx_Read(uart_type device , uint16_t i) {
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    uint8_t index = i % BUFFER_SIZE;
    return rx_buffer[device][index];
}

int Command_tx_GetLength(uart_type device) {
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return (tx_writeindex[device] + BUFFER_SIZE - tx_readindex[device]) % BUFFER_SIZE;
}

int Command_rx_GetLength(uart_type device) {
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return (rx_writeindex[device] + BUFFER_SIZE - rx_readindex[device]) % BUFFER_SIZE;
}

int Command_tx_GetRemain(uart_type device) {
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return BUFFER_SIZE - 1 - Command_tx_GetLength(device);
}

int Command_rx_GetRemain(uart_type device) {
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return BUFFER_SIZE - 1 - Command_rx_GetLength(device);
}

int Command_tx_GetContinuous(uart_type device, uint8_t **ptr)
{
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    if (ptr == NULL ) return EMPTY_DATA;

    uint8_t  len    = Command_tx_GetLength(device);
    uint16_t to_end = BUFFER_SIZE - tx_readindex[device];
    *ptr = &tx_buffer[device][tx_readindex[device]];
    return (len < to_end) ? len : (uint8_t)to_end;
}

int Command_tx_Write(uart_type device , uint8_t *data, uint16_t length) {

    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    if (data == NULL) return EMPTY_DATA;
    if (length ==0) return LENGTH_ERROR;

    if (Command_tx_GetRemain(device) < length) {
        tx_overflow_cnt[device]++;
        return BUFFER_FULL;
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

int Command_rx_Write(uart_type device , uint8_t *data, uint16_t length) {

    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    if (data == NULL) return EMPTY_DATA;
    if (length ==0) return LENGTH_ERROR;

    if (Command_rx_GetRemain(device) < length) {
        rx_overflow_cnt[device]++;
        return BUFFER_FULL;
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

int Command_rx_Peek(uart_type device, uint16_t offset)
{
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return Command_rx_Read(device, rx_readindex[device] + offset);
}

int Command_rx_ReadData(uart_type device, uint8_t *data, uint16_t length)
{
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    if (data == NULL) return EMPTY_DATA;
    if (length ==0) return LENGTH_ERROR;

    if (Command_rx_GetLength(device) < length) {
        length = Command_rx_GetLength(device);
    }

    if (rx_readindex[device] + length < BUFFER_SIZE) {
        memcpy(data, rx_buffer[device] + rx_readindex[device], length);
    } else {
        uint8_t firstLength = BUFFER_SIZE - rx_readindex[device];
        memcpy(data, rx_buffer[device] + rx_readindex[device], firstLength);
        memcpy(data + firstLength, rx_buffer[device], length - firstLength);
    }
    Command_rx_AddReadIndex(device, length);
    return length;
}

int Command_tx_GetOverflow(uart_type device)
{
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return tx_overflow_cnt[device];
}

int Command_rx_GetOverflow(uart_type device)
{
    if (!DEVICE_VALID(device)) return DEVICE_NOT_FOUND;
    return rx_overflow_cnt[device];
}