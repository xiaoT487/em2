
#ifndef MIAO1_USER_CAN_H
#define MIAO1_USER_CAN_H

#define CAN_ID_MAX_NUM 24
#define CAN_BUFFER_SIZE 64

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    can1 ,
    can2 ,
    can3 ,
    CAN_DEVICE_NUM
} can_type ;

typedef enum
{
    LEN_ERROR,
    OK = -1,
    DEVICE_EXCEED_MAX = -2,
    EMPTY_DATA = -3,
    ID_ERROR = -4,
    ID_REPEAT = -5,
    REGISTER_ERROR = -6,
    ADD_MESSAGE_ERROR = -7,
} can_feedback;

typedef void (*can_callback)(can_type device, uint32_t id, const uint8_t *data, size_t len);

void can_init(can_type device);
int can_id_register(can_type device , uint32_t id , can_callback func);
int can_send(can_type device, uint32_t id , const uint8_t *data ,uint32_t len);

#endif //MIAO1_USER_CAN_H
