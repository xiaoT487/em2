#ifndef MIAO1_INS_H
#define MIAO1_INS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    INS_STATUS_CALIBRATING = 0, /* 上电自动校准中 */
    INS_STATUS_MANUAL_CALI = 1, /* 手动校准中 */
    INS_STATUS_READY       = 2, /* 校准完成，正常输出 */
    INS_STATUS_ERROR       = 3, /* BMI088 初始化失败 */
} ins_status_t;

typedef struct
{
    float gyro[3];
    float accel[3];
    float temp;
} ins_raw_data_t;

typedef struct
{
    float q[4];
    float yaw;
    float pitch;
    float roll;
    float yaw_total_angle;
    ins_raw_data_t raw;
} ins_data_t;

void ins_init(void);
void ins_task(void *argument);
void ins_task_start(void);

uint8_t ins_status(void);
uint8_t ins_error_code(void);

/* 线程安全地复制一份数据；不要在中断里调用。 */
void ins_get_data(ins_data_t *out);

/* 调试器里直接看，任务里请用 ins_get_data()。 */
const ins_data_t *ins_data(void);

/* 重新做一次 10000 点的手动零偏校准。 */
void ins_start_manual_calibration(void);

/* 把当前航向置为 0，不改变 q。 */
void ins_zero_yaw(void);

#ifdef __cplusplus
}
#endif

#endif /* MIAO1_INS_H */
