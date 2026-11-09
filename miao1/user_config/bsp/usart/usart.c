
#include "usart.h"

static UART_HandleTypeDef *handle[UART_TYPE_NUM] = {
    [uart1] = &huart1,
    // [uart2] = &huart2,
    // [uart3] = &huart3,
    // [uart5] = &huart5,
    // [uart7] = &huart7,
    // [uart10] = &huart10,
};

static uint8_t tx_busy[UART_TYPE_NUM];
static uint8_t rx_busy[UART_TYPE_NUM];


void UART_send(uart_type device , uint8_t *data , uint16_t length)
{
    HAL_UART_Transmit(handle[device], (uint8_t *)(&data), length, HAL_MAX_DELAY);
}

void UART_send_async(uart_type device , uint8_t data , uint16_t length)
{
    if (!tx_busy[device])
    {
        tx_busy[device] = 1;
        HAL_UART_Transmit_DMA(handle[device], (uint8_t *)(&data), length);
    } else
    {
        if (Command_tx_GetRemain(device)>=length)
        Command_tx_Write(device, &data, length);
    }

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h)
{
    for (int i=0;i<UART_TYPE_NUM;i++)
    {
        if (handle[i] == h)
        {
            if (Command_tx_GetLength(i) == 0)
            {
                tx_busy[i] = 0;
                break;
            } else
            {
                int buff=Command_tx_Read_one(i);
                Command_tx_AddReadIndex(i,1);
                HAL_UART_Transmit_DMA(h,(uint8_t *)&buff,1);
            }


        }
    }
}