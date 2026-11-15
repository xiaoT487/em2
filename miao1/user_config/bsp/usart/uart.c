
#include "uart.h"

static UART_HandleTypeDef *handle[UART_TYPE_NUM] = {
    [uart1] = &huart1,
    [uart2] = &huart2,
    [uart3] = &huart3,
    [uart5] = &huart5,
    [uart7] = &huart7,
    [uart10] = &huart10,
};

__attribute__((section(".RAM_D1"), aligned(32)))
static uint8_t rx_dma[UART_TYPE_NUM][RX_DMA_SIZE];
static volatile uint8_t tx_busy[UART_TYPE_NUM];
static volatile uint8_t tx_len[UART_TYPE_NUM];

void uart_init(void)
{
    for (uint8_t i = 0; i < UART_TYPE_NUM; i++) {
        if (handle[i] != NULL) {
            HAL_UART_Init(handle[i]);
            UART_receive(i);
        }
    }
}

static uart_type UART_get_device(UART_HandleTypeDef *h)
{
    for (int i=0;i<UART_TYPE_NUM;i++)
    {
        if (handle[i] == h) return i;
    }
    return UART_TYPE_NUM;
}

void UART_send(uart_type device , uint8_t *data , uint16_t length)
{
    if (device >= UART_TYPE_NUM || handle[device] == NULL || data == NULL || length == 0) return;
    HAL_UART_Transmit(handle[device], data , length, HAL_MAX_DELAY);
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

    SCB_CleanDCache_by_Addr((uint32_t *)p, len);

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
        Command_tx_Write(device, data, length);
    }

    if (!tx_busy[device]) {
        UART_start_tx(device);
    }
    __set_PRIMASK(primask);
}

void UART_receive(uart_type device)
{
    if (device >= UART_TYPE_NUM || handle[device] == NULL) return;
    if (HAL_UARTEx_ReceiveToIdle_DMA(handle[device], rx_dma[device], RX_DMA_SIZE) == HAL_OK) {
        __HAL_DMA_DISABLE_IT(handle[device]->hdmarx, DMA_IT_HT);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *h, uint16_t Size)
{
    uart_type device = UART_get_device(h);
    if (device == UART_TYPE_NUM) return;

    SCB_InvalidateDCache_by_Addr((uint32_t *)rx_dma[device], Size);

    Command_rx_Write(device, rx_dma[device], Size);
    UART_receive(device);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h)
{
    uart_type device = UART_get_device(h);
    if (device == UART_TYPE_NUM) return;

    Command_tx_AddReadIndex(device, tx_len[device]);
    UART_start_tx(device);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *h)
{
    uart_type device = UART_get_device(h);
    if (device == UART_TYPE_NUM) return;

    __HAL_UART_CLEAR_FLAG(h, UART_FLAG_ORE | UART_FLAG_FE | UART_FLAG_PE | UART_FLAG_NE);

    if (tx_busy[device] && h->gState == HAL_UART_STATE_READY) {
        UART_start_tx(device);
    }
    if (h->RxState == HAL_UART_STATE_READY) {
        UART_receive(device);
    }
}