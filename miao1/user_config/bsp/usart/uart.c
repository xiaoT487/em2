
#include "uart.h"

static UART_HandleTypeDef *handle[UART_TYPE_NUM] = {
    [uart1] = &huart1,
    // [uart2] = &huart2,
    // [uart3] = &huart3,
    // [uart5] = &huart5,
    // [uart7] = &huart7,
    // [uart10] = &huart10,
};

static volatile uint8_t tx_busy[UART_TYPE_NUM];
static volatile uint8_t tx_len[UART_TYPE_NUM];
static volatile uint8_t rx_busy[UART_TYPE_NUM];


void UART_send(uart_type device , uint8_t *data , uint16_t length)
{
    if (device >= UART_TYPE_NUM || handle[device] == NULL || data == NULL) return;
    HAL_UART_Transmit(handle[device], (uint8_t *)(&data), length, HAL_MAX_DELAY);
}

static void UART_start_tx(uart_type device)
{
    uint8_t *p;
    uint8_t len = Command_tx_GetContinuous(device, &p);

    if (len == 0) {
        tx_busy[device] = 0;
        return;
    }

    tx_len[device] = len;
    if (HAL_UART_Transmit_DMA(handle[device], p, len) == HAL_OK) {
        tx_busy[device] = 1;
    } else {
        tx_busy[device] = 0;
    }
}


void UART_send_async(uart_type device , uint8_t *data , uint16_t length)
{
    if (device >= UART_TYPE_NUM || handle[device] == NULL || data == NULL || length == 0) return;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (Command_tx_GetRemain(device) >= length) {
        Command_tx_Write(device, data, (uint8_t)length);
    }

    if (!tx_busy[device]) {
        UART_start_tx(device);
    }
    __set_PRIMASK(primask);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h)
{
    for (int i=0;i<UART_TYPE_NUM;i++)
    {
        if (handle[i] == h)
        {
            Command_tx_AddReadIndex(i, tx_len[i]);
            UART_start_tx(i); 
            break;
        }
    }
}