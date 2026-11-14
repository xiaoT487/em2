
#include "user_can.h"

#include <string.h>

#include "fdcan.h"
#include "stm32h7xx_hal_fdcan.h"

static FDCAN_HandleTypeDef *handle[CAN_DEVICE_NUM] = {
    [can1] = &hfdcan1 ,
    [can2] = &hfdcan2 ,
    [can3] = &hfdcan3
};

static uint8_t id_cnt[CAN_DEVICE_NUM];
static uint32_t can_id[CAN_DEVICE_NUM][CAN_ID_MAX_NUM];
static can_callback callback[CAN_DEVICE_NUM][CAN_ID_MAX_NUM];
uint8_t can_rx_buffer[CAN_DEVICE_NUM][CAN_ID_MAX_NUM][CAN_BUFFER_SIZE];

void can_init(can_type device) {
    HAL_FDCAN_ActivateNotification(handle[device], FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    HAL_FDCAN_ActivateNotification(handle[device], FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
    HAL_FDCAN_ConfigGlobalFilter(handle[device], FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    HAL_FDCAN_Start(handle[device]);
}

int can_id_register(can_type device , uint32_t id , can_callback func)
{
    if (id_cnt[device] >= CAN_ID_MAX_NUM )
        return DEVICE_EXCEED_MAX;
    if (func == NULL)
        return EMPTY_DATA;
    if (id > 0x7ff)
        return ID_ERROR;

    for (int j=0;j < id_cnt[device]; j++)
    {
        if (can_id[device][j] == id)
            return ID_REPEAT;
    }

    can_id[device][id_cnt[device]] = id;
    callback[device][id_cnt[device]] = func;

    FDCAN_FilterTypeDef filter = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = id_cnt[device],
        .FilterType = FDCAN_FILTER_MASK,
        .FilterID1 = id,
        .FilterID2 = 0x7ff,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
    };

    if(HAL_FDCAN_ConfigFilter(handle[device], &filter) == HAL_OK)
    {
        id_cnt[device]++;
        return OK;
    }
    can_id[device][id_cnt[device]] = 0;
    callback[device][id_cnt[device]] = 0;
    return REGISTER_ERROR;
}

static uint32_t len2code(uint8_t l) {
    if(l <= 8) return l;
    if(l == 12) return FDCAN_DLC_BYTES_12;
    if(l == 16) return FDCAN_DLC_BYTES_16;
    if(l == 20) return FDCAN_DLC_BYTES_20;
    if(l == 24) return FDCAN_DLC_BYTES_24;
    if(l == 32) return FDCAN_DLC_BYTES_32;
    if(l == 48) return FDCAN_DLC_BYTES_48;
    if(l == 64) return FDCAN_DLC_BYTES_64;
    return LEN_ERROR;
}

static uint8_t code2len(uint32_t l) {
    if(l <= 8) return l;
    if(l == FDCAN_DLC_BYTES_12) return 12;
    if(l == FDCAN_DLC_BYTES_16) return 16;
    if(l == FDCAN_DLC_BYTES_20) return 20;
    if(l == FDCAN_DLC_BYTES_24) return 24;
    if(l == FDCAN_DLC_BYTES_32) return 32;
    if(l == FDCAN_DLC_BYTES_48) return 48;
    if(l == FDCAN_DLC_BYTES_64) return 64;
    return LEN_ERROR;
}

int can_send(can_type device, uint32_t id , const uint8_t *data ,uint32_t len)
{
    if (data == NULL)
        return EMPTY_DATA;
    if (len <= 0 || len > 64 || len2code(len) == LEN_ERROR)
        return LEN_ERROR;
    if (id > 0x7ff)
        return ID_ERROR;

    FDCAN_TxHeaderTypeDef header = {
        .Identifier = id,
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = len2code(len),
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = len > 8 ? FDCAN_BRS_ON : FDCAN_BRS_OFF,
        .FDFormat = len > 8 ? FDCAN_FD_CAN : FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
        .MessageMarker = 0x01
    };

    if (HAL_FDCAN_AddMessageToTxFifoQ(handle[device], &header, data) == HAL_ERROR)
    return ADD_MESSAGE_ERROR;

    return OK;
}

static void bsp_can_callback_sol(can_type device, uint32_t fifo) {
    FDCAN_RxHeaderTypeDef header;
    static uint8_t buf[CAN_BUFFER_SIZE] = { 0 };
    while (HAL_FDCAN_GetRxFifoFillLevel(handle[device], fifo)) {
        if (HAL_FDCAN_GetRxMessage(handle[device], fifo, &header, buf) != HAL_OK) break;
        for (uint8_t i = 0; i < id_cnt[device]; i++) {
            if (can_id[device][i] == header.Identifier) {
                uint8_t len = code2len(header.DataLength);
                memcpy(can_rx_buffer[device][i], buf, len);
                if (callback[device][i] != NULL) callback[device][i](device, header.Identifier, can_rx_buffer[device][i], len);
                break;
            }
        }
    }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *device, uint32_t RxFifo0ITs) {
    UNUSED(RxFifo0ITs);
    switch((uint32_t) device->Instance) {
    case (uint32_t) FDCAN1:
        bsp_can_callback_sol(can1, FDCAN_RX_FIFO0); break;
    case (uint32_t) FDCAN2:
        bsp_can_callback_sol(can2, FDCAN_RX_FIFO0); break;
    case (uint32_t) FDCAN3:
        bsp_can_callback_sol(can3, FDCAN_RX_FIFO0); break;
    default: break;
    }
}