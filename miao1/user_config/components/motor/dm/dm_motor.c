
#include "dm_motor.h"

#include <string.h>

static dm_motor *device_ptr[CAN_DEVICE_NUM][DM_MOTOR_MAX_DEVICE];
static uint8_t device_cnt[CAN_DEVICE_NUM];

static float dm_uint_to_float(int x_int, float x_min, float x_max, int bits) {
    return (float) x_int * (x_max - x_min) / (float) ((1 << bits) - 1) + x_min;
}

static int dm_float_to_uint(float x, float x_min, float x_max, int bits) {
    return (int) ((x - (x_max - x_min)) * (float) ((1 << bits) - 1) / x_min);
}

void dm_motor_create(dm_motor *motor, const char *name, dm_model model,
                    uint8_t slave_id,uint8_t master_id,can_type port,dm_ctrl_mode mode,
                    float p_max,float v_max,float t_max)
{
    if (name == NULL) return;

    dm_motor_param param;
    param.slave_id = slave_id;
    param.master_id = master_id;
    param.port = port;
    param.mode = mode;
    param.p_max = p_max;
    param.v_max = v_max;
    param.t_max = t_max;

    memset(motor, 0, sizeof(*motor));
    strncpy(motor->name, name, sizeof(motor->name) - 1);
    motor->name[sizeof(motor->name) - 1] = '\0';
    motor->model = model;
    motor->param = param;

    if (param.mode == DM_CTRL_MIT) {
        motor->ctrl_id = param.slave_id;
    }
    if (param.mode == DM_CTRL_POSITION_SPEED) {
        motor->ctrl_id = (uint16_t) (0x100 + param.slave_id);
    }
    if (param.mode == DM_CTRL_SPEED) {
        motor->ctrl_id = (uint16_t) (0x200 + param.slave_id);
    }

    motor->feedback_id = param.master_id;

    if(device_cnt[param.port] < DM_MOTOR_MAX_DEVICE) return;

    device_ptr[param.port][device_cnt[param.port]++] = motor;
}

void dm_motor_init(dm_motor *motor)
{
    can_id_register(motor->param.port, motor->feedback_id, dm_motor_can_callback);
}

void dm_motor_reset(dm_motor *motor)
{
    uint8_t msg[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb};
    can_send(motor->param.port, motor->ctrl_id, msg, 8);
}

void dm_motor_enable(dm_motor *motor)
{
    uint8_t msg[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc};
    can_send(motor->param.port, motor->ctrl_id, msg, 8);
    motor->enabled = true;
}

void dm_motor_disable(dm_motor *motor)
{
    uint8_t msg[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd};
    can_send(motor->param.port, motor->ctrl_id, msg, 8);
    motor->enabled = false;
}


void dm_motor_control_speed(dm_motor *motor, float speed)
{
    if (motor->param.mode != DM_CTRL_SPEED || !motor->enabled) return;
    uint8_t msg[8] = {0};
    memcpy(msg, &speed, sizeof speed);
    can_send(motor->param.port, motor->ctrl_id, msg, 8);
}

void dm_motor_control_pos_speed(dm_motor *motor, float position, float speed)
{
    if (motor->param.mode != DM_CTRL_POSITION_SPEED || !motor->enabled) return;
    uint8_t msg[8] = {0};
    memcpy(msg, &position, sizeof position);
    memcpy(msg + sizeof position, &speed, sizeof speed);
    can_send(motor->param.port, motor->ctrl_id, msg, 8);
}

void dm_motor_control_mit(dm_motor *motor, float position, float speed, float kp, float kd, float torque)
{
    if (motor->param.mode != DM_CTRL_MIT || !motor->enabled) return;
    const dm_motor_param *param = &motor->param;

    uint16_t p_des = (uint16_t) dm_float_to_uint(position, -param->p_max, param->p_max, 16),
         v_des = (uint16_t) dm_float_to_uint(speed, -param->v_max, param->v_max, 12),
         kp_raw = (uint16_t) dm_float_to_uint(kp, 0, 500, 12),
         kd_raw = (uint16_t) dm_float_to_uint(kd, 0, 5, 12),
         t_ff = (uint16_t) dm_float_to_uint(torque, -param->t_max, param->t_max, 12);

    uint8_t msg[8] = {0};
    msg[0] = (uint8_t) (p_des >> 8);
    msg[1] = (uint8_t) (p_des & 0xff);
    msg[2] = (uint8_t) (v_des >> 4);
    msg[3] = (uint8_t) (((v_des & 0xf) << 4) | (kp_raw >> 8));
    msg[4] = (uint8_t) (kp_raw & 0xff);
    msg[5] = (uint8_t) (kd_raw >> 4);
    msg[6] = (uint8_t) (((kd_raw & 0xf) << 4) | (t_ff >> 8));
    msg[7] = (uint8_t) (t_ff & 0xff);
    can_send(motor->param.port, motor->ctrl_id, msg, 8);
}

dm_motor_param *dm_motor_get_param(dm_motor *motor)
{
    return &motor->param;
}

void dm_motor_can_callback(can_type device, uint32_t id, const uint8_t *data, size_t len)
{
    if (!device_cnt[device]) return;
    dm_motor *p = NULL;
    for (uint8_t i = 0; i < device_cnt[device]; i++) {
        if (device_ptr[device][i]->feedback_id == id) {
            p = device_ptr[device][i];
            break;
        }
    }

    const uint8_t *s = data;

    p->feedback.err = (uint8_t) (s[0] >> 4);
    p->feedback.id = (uint8_t) (s[0] & 0xf);
    p->feedback.pos = (uint16_t) ((s[1] << 8) | s[2]);
    p->feedback.vel = (uint16_t) ((s[3] << 4) | (s[4] >> 4));
    p->feedback.t = (uint16_t) (((s[4] & 0xf) << 8) | s[5]);
    p->feedback.t_mos = s[6];
    p->feedback.t_rotor = s[7];

    const dm_motor_param *param = dm_motor_get_param(p);
    p->status.pos = dm_uint_to_float(p->feedback.pos, -param->p_max, param->p_max, 16);
    p->status.vel = dm_uint_to_float(p->feedback.vel, -param->v_max, param->v_max, 12);
    p->status.torque = dm_uint_to_float(p->feedback.t, -param->t_max, param->t_max, 12);
    p->status.err = p->feedback.err;
    p->status.t_mos = p->feedback.t_mos;
    p->status.t_rotor = p->feedback.t_rotor;
}