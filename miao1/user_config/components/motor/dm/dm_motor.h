
#ifndef MIAO1_DM_MOTOR_H
#define MIAO1_DM_MOTOR_H

#define DM_MOTOR_MAX_DEVICE 4
#include <stdbool.h>
#include <stdint.h>

#include "bsp/can/user_can.h"

typedef enum {
    DM_CTRL_MIT ,
    DM_CTRL_POSITION_SPEED ,
    DM_CTRL_SPEED ,
} dm_ctrl_mode;

typedef enum {
    DM_MODEL_J4310 ,
    DM_MODEL_J8009P ,
} dm_model;

typedef struct {
    uint8_t slave_id;
    uint8_t master_id;
    can_type port;
    dm_ctrl_mode mode;

    float p_max;
    float v_max;
    float t_max;
} dm_motor_param;

typedef struct {
    uint8_t err;
    float pos;
    float vel;
    float torque;
    float t_mos;
    float t_rotor;
    uint32_t last_online_time;
} dm_motor_status;

typedef struct {
    uint8_t id;
    uint8_t err;

    uint16_t pos;
    uint16_t vel;
    uint16_t t;

    uint8_t t_mos;
    uint8_t t_rotor;
} dm_motor_feedback;

typedef struct motor_dm dm_motor;

struct motor_dm
{
    dm_motor_status status;

    bool enabled;

    uint16_t ctrl_id;
    uint16_t feedback_id;
    dm_motor_feedback feedback;
    char name[128];

    dm_model model;
    dm_motor_param param;
};

void dm_motor_create(dm_motor *motor, const char *name, dm_model model,
                    uint8_t slave_id,uint8_t master_id,can_type port,dm_ctrl_mode mode,
                    float p_max,float v_max,float t_max);
void dm_motor_init(dm_motor *motor);
void dm_motor_reset(dm_motor *motor);
void dm_motor_enable(dm_motor *motor);
void dm_motor_disable(dm_motor *motor);
void dm_motor_control_speed(dm_motor *motor, float speed);
void dm_motor_control_pos_speed(dm_motor *motor, float position, float speed);
void dm_motor_control_mit(dm_motor *motor, float position, float speed, float kp, float kd, float torque);
dm_motor_param *dm_motor_get_param(dm_motor *motor);
void dm_motor_can_callback(can_type device, uint32_t id, const uint8_t *data, size_t len);

#endif //MIAO1_DM_MOTOR_H
