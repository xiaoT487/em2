
#include "dji_motor.h"

#include <string.h>

#include "cmsis_os2.h"

static const uint16_t ctrl_id_map[] = {0x2ff, 0x1ff, 0x2fe, 0x1fe, 0x200};
enum { CTRL_ID_MAP_SIZE = sizeof(ctrl_id_map) / sizeof(ctrl_id_map[0]) };

static uint8_t id_place_trans(uint16_t x)
{
    for (int i = 0; i < CTRL_ID_MAP_SIZE; i++)
        if (ctrl_id_map[i] == x)
            return (uint8_t)i;
    return 0;
}

static int dji_motor_init_ok = 0;
static dji_motor *device_ptr[CAN_DEVICE_NUM][DJI_MOTOR_MAX_DEVICE];
static uint8_t device_cnt[CAN_DEVICE_NUM];

static uint8_t dji_can_tx_buf[CAN_DEVICE_NUM][CTRL_ID_MAP_SIZE][8];
static uint8_t ctrl_id_used[CAN_DEVICE_NUM][CTRL_ID_MAP_SIZE]={0};

void dji_motor_create(dji_motor *motor, const char *name, dji_model model, const uint8_t id,can_type can_port,dji_ctrl_mode mode)
{
    if (id <1 || id>8 || name == NULL || (model != DJI_MODEL_GM6020 && model != DJI_MODEL_M2006 && model != DJI_MODEL_M3508))
        return;

    dji_motor_param param;
    param.id =id;
    param.mode = mode;
    param.can_port = can_port;

    memset(motor, 0, sizeof(*motor));
    strncpy(motor->name, name, sizeof(motor->name) - 1);
    motor->name[sizeof(motor->name) - 1] = '\0';

    motor->model = model;
    motor->param = param;

    if (model == DJI_MODEL_GM6020)
    {
        if (param.mode == DJI_CTRL_VOLTAGE) {
            motor->ctrl_id = param.id < 5 ? 0x1ff : 0x2ff;
        }
        if (param.mode == DJI_CTRL_CURRENT) {
            motor->ctrl_id = param.id < 5 ? 0x1fe : 0x2fe;
        }
        motor->feedback_id = (uint16_t)(0x204 + param.id);
    }

    if (model == DJI_MODEL_M3508 || model == DJI_MODEL_M2006)
    {
        motor->ctrl_id = param.id < 5 ? 0x200 : 0x1ff;
        motor->feedback_id = (uint16_t)(0x200 + param.id);
    }

    device_ptr[param.can_port][device_cnt[param.can_port]++] = motor;
    ctrl_id_used[param.can_port][id_place_trans(motor->ctrl_id)] = 1;
}

static void dji_motor_enable(dji_motor *motor)
{
    if (motor->enabled) return;
    motor->enabled = true;
}

void dji_motor_init(dji_motor *motor)
{
    if (!dji_motor_init_ok) dji_motor_init_ok = 1;

    can_id_register(motor->param.can_port, motor->feedback_id, dji_motor_can_callback);
    dji_motor_enable(motor);
}

void dji_motor_update(dji_motor *motor, float output)
{
    if (!motor->enabled) return;
    motor->output = output;

    uint8_t cid = id_place_trans(motor->ctrl_id);
    uint8_t mid = motor->param.id < 5 ? motor->param.id : (uint8_t)(motor->param.id - 4);
    int16_t val = (int16_t)output;

    dji_can_tx_buf[motor->param.can_port][cid][2 * mid - 2] = (uint8_t)(val >> 8);
    dji_can_tx_buf[motor->param.can_port][cid][2 * mid - 1] = (uint8_t)(val & 0xff);

}

static void dji_motor_clear(dji_motor *motor)
{
    dji_motor_update(motor, 0);
    motor->status.angle = 0;
    motor->status.current = 0;
    motor->status.speed = 0;
    motor->status.temperature = 0;
}

static void dji_motor_disable(dji_motor *motor)
{
    if (!motor->enabled) return;
    dji_motor_clear(motor);
    motor->enabled = false;
}

void dji_motor_can_callback(can_type device, uint32_t id, const uint8_t *data, size_t len)
{
    if (!device_cnt[device]) return;

    dji_motor *p = NULL;
    for (uint8_t i = 0; i < device_cnt[device]; i++)
    {
        if (device_ptr[device][i]->feedback_id == id)
        {
            p = device_ptr[device][i];
            break;
        }
    }

    const uint8_t *s = data;

    p->feedback.angle = (int16_t)((s[0] << 8) | s[1]);
    p->feedback.speed = (int16_t)((s[2] << 8) | s[3]);
    p->feedback.current = (int16_t)((s[4] << 8) | s[5]);
    p->feedback.temp = s[6];

    p->status.angle = (float)p->feedback.angle;
    p->status.speed = (float)p->feedback.speed;
    p->status.current = (float)p->feedback.current;
    p->status.temperature = (float)p->feedback.temp;

}

void dji_motor_send(void)
{
    while (!dji_motor_init_ok)  osDelay(10);

        for (uint8_t i = 0; i < CAN_DEVICE_NUM; i++)
        {
            if (!device_cnt[i]) continue;
            // Send Control Message
            for (uint8_t j = 0; j < CTRL_ID_MAP_SIZE; j++)
            {
                if (ctrl_id_used[i][j])
                {
                    can_send((can_type)i, ctrl_id_map[j], dji_can_tx_buf[i][j], 8);
                }
            }
        }
}