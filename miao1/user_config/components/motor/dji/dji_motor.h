
#ifndef MIAO1_DJI_MOTOR_H
#define MIAO1_DJI_MOTOR_H

#define DJI_MOTOR_MAX_DEVICE 4
#include <stdbool.h>
#include <stdint.h>

#include "bsp/can/user_can.h"

typedef enum {
    DJI_CTRL_VOLTAGE ,
    DJI_CTRL_CURRENT ,
} dji_ctrl_mode;

typedef enum {
    DJI_MODEL_GM6020 ,
    DJI_MODEL_M3508 ,
    DJI_MODEL_M2006 ,
} dji_model;

typedef struct {
    uint8_t id;
    can_type can_port;
    dji_ctrl_mode mode;
} dji_motor_param;

typedef struct {
    int16_t angle;
    int16_t speed;
    int16_t current;
    uint8_t temp;
} dji_motor_feedback;

typedef struct {
    double speed;
    double angle;
    double current;
    double torque;
    double temperature;
    uint32_t last_online_time;
} dji_motor_status;

typedef struct motor_dji dji_motor;


struct motor_dji {
    dji_motor_status status;
    bool enabled;

    float output;
    uint16_t ctrl_id;
    uint16_t feedback_id;
    dji_motor_feedback feedback;
    char name[128];

    dji_model model;
    dji_motor_param param;
};

void dji_motor_create(dji_motor *motor, const char *name, dji_model model, const uint8_t id,can_type can_port,dji_ctrl_mode mode);
void dji_motor_init(dji_motor *motor);
void dji_motor_update(dji_motor *motor, float output);
void dji_motor_can_callback(can_type device, uint32_t id, const uint8_t *data, size_t len);
void dji_motor_send(void);

#endif //MIAO1_DJI_MOTOR_H
