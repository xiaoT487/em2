#include "ins.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

#include "uart.h"
#include "bsp/imu/imu.h"
#include "bsp/imu/bmi088/BMI088driver.h"

/* ---------------- 可裁剪配置 ---------------- */

#ifndef INS_ENABLE_CONSOLE
#define INS_ENABLE_CONSOLE 1
#endif

#ifndef INS_CONSOLE_UART
#define INS_CONSOLE_UART uart1
#endif

#define INS_TASK_PERIOD_MS       1u
#define INS_AUTO_CALI_SAMPLES    1500u
#define INS_MANUAL_CALI_SAMPLES  10000u
#define INS_UART_PERIOD_MS       100u
#define INS_CONSOLE_PERIOD_MS    10u

#define INS_DT                   0.001f
#define INS_RAD_TO_DEG           57.2957795f
#define INS_DEG_TO_RAD           0.0174532925f
#define INS_MAHONY_KP            2.5f
#define INS_ACCEL_LPF_ALPHA      0.05f
#define INS_ACCEL_NORM_MIN       8.0f
#define INS_ACCEL_NORM_MAX       11.5f

/* ---------------- 模块状态 ---------------- */

static volatile uint8_t  s_inited;
static volatile uint8_t  s_status = INS_STATUS_ERROR;
static volatile uint8_t  s_error_code;
static volatile uint32_t s_cal_count;
static volatile uint32_t s_cal_target = INS_AUTO_CALI_SAMPLES;

static osThreadId_t s_task;
static const osThreadAttr_t s_task_attr = {
    .name = "ins",
    .stack_size = 512 * 4,
    .priority = (osPriority_t)osPriorityAboveNormal,
};

static ins_data_t s_data;
static ins_data_t s_pub;
static float s_gyro_sum[3];
static float s_gyro_bias[3];

/* 姿态：Mahony 互补滤波，q 为机体到世界的四元数。 */
static float   s_q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
static float   s_accel_lpf[3];
static uint8_t s_attitude_seeded;
static float   s_yaw_offset;
static float   s_yaw_last;
static int16_t s_yaw_round;

#if INS_ENABLE_CONSOLE
static volatile uint8_t s_watch;
static volatile uint8_t s_test_active;
static uint8_t  s_test_index;
static float    s_test_sum;
static float    s_test_start_yaw;
static uint32_t s_test_next_tick;

static uint8_t  s_line_len;
static char     s_line[48];
#endif

_Static_assert(sizeof(imu_data) == sizeof(ins_raw_data_t),
               "imu_data layout must match ins_raw_data_t");

/* ---------------- 小工具 ---------------- */

static void ins_reset_attitude(void)
{
    s_q[0] = 1.0f;
    s_q[1] = 0.0f;
    s_q[2] = 0.0f;
    s_q[3] = 0.0f;

    s_data.q[0] = 1.0f;
    s_data.q[1] = 0.0f;
    s_data.q[2] = 0.0f;
    s_data.q[3] = 0.0f;

    s_data.roll = 0.0f;
    s_data.pitch = 0.0f;
    s_data.yaw = 0.0f;
    s_data.yaw_total_angle = 0.0f;

    s_accel_lpf[0] = 0.0f;
    s_accel_lpf[1] = 0.0f;
    s_accel_lpf[2] = 0.0f;
    s_attitude_seeded = 0;
    s_yaw_offset = 0.0f;
    s_yaw_last = 0.0f;
    s_yaw_round = 0;
}

static float ins_wrap_180(float angle)
{
    while (angle > 180.0f)
    {
        angle -= 360.0f;
    }
    while (angle < -180.0f)
    {
        angle += 360.0f;
    }
    return angle;
}

static void ins_quat_normalize(float *q)
{
    float n = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (n < 1.0e-6f)
    {
        q[0] = 1.0f;
        q[1] = 0.0f;
        q[2] = 0.0f;
        q[3] = 0.0f;
        return;
    }
    float inv = 1.0f / n;
    q[0] *= inv;
    q[1] *= inv;
    q[2] *= inv;
    q[3] *= inv;
}

static void ins_quat_from_roll_pitch(float roll_deg, float pitch_deg)
{
    float half_roll  = roll_deg * INS_DEG_TO_RAD * 0.5f;
    float half_pitch = pitch_deg * INS_DEG_TO_RAD * 0.5f;

    float cr = cosf(half_roll);
    float sr = sinf(half_roll);
    float cp = cosf(half_pitch);
    float sp = sinf(half_pitch);

    s_q[0] = cr * cp;
    s_q[1] = sr * cp;
    s_q[2] = cr * sp;
    s_q[3] = -sr * sp;
    ins_quat_normalize(s_q);
}

/* ---------------- 控制台 ---------------- */

#if INS_ENABLE_CONSOLE
static void ins_send(const char *text)
{
    if (text == NULL)
    {
        return;
    }
    /* 用 DMA 队列发送，避免 1 kHz 任务被串口阻塞。 */
    UART_send_async(INS_CONSOLE_UART, (uint8_t *)text, (uint16_t)strlen(text));
}

static void ins_send_status_line(void)
{
    char buf[160];
    long roll  = (long)(s_data.roll * 1000.0f);
    long pitch = (long)(s_data.pitch * 1000.0f);
    long yaw   = (long)(s_data.yaw * 1000.0f);
    long ytot  = (long)(s_data.yaw_total_angle * 1000.0f);
    long temp  = (long)(s_data.raw.temp * 100.0f);
    long gx    = (long)((s_data.raw.gyro[0] - s_gyro_bias[0]) * 1000.0f);
    long gy    = (long)((s_data.raw.gyro[1] - s_gyro_bias[1]) * 1000.0f);
    long gz    = (long)((s_data.raw.gyro[2] - s_gyro_bias[2]) * 1000.0f);

    snprintf(buf, sizeof(buf),
             "ins,r=%ld,p=%ld,y=%ld,yt=%ld,t=%ld,gx=%ld,gy=%ld,gz=%ld,st=%u\r\n",
             roll, pitch, yaw, ytot, temp, gx, gy, gz, (unsigned)s_status);
    ins_send(buf);
}

static void ins_print_help(void)
{
    /* UART 发送队列只有 256 字节，帮助信息分两条发，避免整条被丢弃。 */
    ins_send("ins: cali watch test config zero status help\r\n");
    ins_send("cali=10000 samples, keep still about 10s\r\n");
}

static void ins_start_test(void)
{
    if (s_status != INS_STATUS_READY)
    {
        ins_send("ins: not ready, run cali first\r\n");
        return;
    }

    s_test_active = 1;
    s_test_index = 0;
    s_test_sum = 0.0f;
    s_test_start_yaw = s_data.yaw_total_angle;
    s_test_next_tick = osKernelGetTickCount() + 5000u;
    ins_send("ins: test start, keep the board still (5 x 5 s)\r\n");
}

static void ins_test_poll(void)
{
    if (!s_test_active)
    {
        return;
    }

    uint32_t now = osKernelGetTickCount();
    if ((int32_t)(now - s_test_next_tick) < 0)
    {
        return;
    }

    float yaw_now = s_data.yaw_total_angle;
    float delta = yaw_now - s_test_start_yaw;
    float rate = delta * 12.0f; /* 5 s 积分换算成 deg/min */

    s_test_sum += rate;
    s_test_index++;
    s_test_start_yaw = yaw_now;
    s_test_next_tick = now + 5000u;

    char buf[80];
    snprintf(buf, sizeof(buf), "test #%u = %ld mdeg/min\r\n",
             (unsigned)s_test_index, (long)(rate * 1000.0f));
    ins_send(buf);

    if (s_test_index >= 5u)
    {
        float avg = s_test_sum / 5.0f;
        snprintf(buf, sizeof(buf), "avg = %ld mdeg/min\r\n", (long)(avg * 1000.0f));
        ins_send(buf);
        s_test_active = 0;
    }
}

static void ins_console_exec(char *cmd)
{
    while (*cmd == ' ' || *cmd == '\t')
    {
        cmd++;
    }
    if (*cmd == '\0')
    {
        return;
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0)
    {
        ins_print_help();
    }
    else if (strcmp(cmd, "cali") == 0)
    {
        ins_start_manual_calibration();
        if (s_status == INS_STATUS_MANUAL_CALI)
        {
            ins_send("ins: manual cali started, 10000 samples, keep still\r\n");
        }
        else
        {
            ins_send("ins: cali rejected, check status\r\n");
        }
    }
    else if (strcmp(cmd, "watch") == 0)
    {
        s_watch = (uint8_t)(!s_watch);
        ins_send(s_watch ? "ins: watch on (10 Hz)\r\n" : "ins: watch off\r\n");
    }
    else if (strcmp(cmd, "test") == 0)
    {
        ins_start_test();
    }
    else if (strcmp(cmd, "config") == 0)
    {
        char buf[96];
        snprintf(buf, sizeof(buf), "bias = %ld, %ld, %ld mrad/s\r\n",
                 (long)(s_gyro_bias[0] * 1000.0f),
                 (long)(s_gyro_bias[1] * 1000.0f),
                 (long)(s_gyro_bias[2] * 1000.0f));
        ins_send(buf);
    }
    else if (strcmp(cmd, "zero") == 0)
    {
        if (s_status == INS_STATUS_READY)
        {
            ins_zero_yaw();
            ins_send("ins: yaw zeroed\r\n");
        }
        else
        {
            ins_send("ins: not ready\r\n");
        }
    }
    else if (strcmp(cmd, "status") == 0)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "ins: status=%u error=%u\r\n",
                 (unsigned)s_status, (unsigned)s_error_code);
        ins_send(buf);
    }
    else
    {
        ins_send("ins: unknown command, type help\r\n");
    }
}

static void ins_console_poll(void)
{
    uint8_t ch;

    while (Command_rx_GetLength(INS_CONSOLE_UART) > 0u)
    {
        if (Command_rx_ReadData(INS_CONSOLE_UART, &ch, 1u) != 1u)
        {
            break;
        }

        if (ch == '\r' || ch == '\n')
        {
            if (s_line_len > 0u)
            {
                s_line[s_line_len] = '\0';
                ins_console_exec(s_line);
                s_line_len = 0u;
            }
        }
        else if (ch == 0x08u || ch == 0x7Fu)
        {
            if (s_line_len > 0u)
            {
                s_line_len--;
            }
        }
        else if (s_line_len + 1u < sizeof(s_line))
        {
            s_line[s_line_len++] = (char)ch;
        }
    }
}
#endif /* INS_ENABLE_CONSOLE */

/* ---------------- 校准 ---------------- */

static void ins_finish_calibration(void)
{
    if (s_cal_count == 0u)
    {
        s_status = INS_STATUS_ERROR;
        s_error_code = 0xFEu;
        return;
    }

    float inv = 1.0f / (float)s_cal_count;
    s_gyro_bias[0] = s_gyro_sum[0] * inv;
    s_gyro_bias[1] = s_gyro_sum[1] * inv;
    s_gyro_bias[2] = s_gyro_sum[2] * inv;

    s_data.raw.gyro[0] = 0.0f;
    s_data.raw.gyro[1] = 0.0f;
    s_data.raw.gyro[2] = 0.0f;

    ins_reset_attitude();
    s_status = INS_STATUS_READY;

#if INS_ENABLE_CONSOLE
    {
        char buf[112];
        snprintf(buf, sizeof(buf), "cal done: %ld, %ld, %ld mrad/s\r\n",
                 (long)(s_gyro_bias[0] * 1000.0f),
                 (long)(s_gyro_bias[1] * 1000.0f),
                 (long)(s_gyro_bias[2] * 1000.0f));
        ins_send(buf);
    }
#endif
}

void ins_start_manual_calibration(void)
{
    if (!s_inited || s_status == INS_STATUS_ERROR)
    {
        return;
    }

    taskENTER_CRITICAL();
    s_gyro_sum[0] = 0.0f;
    s_gyro_sum[1] = 0.0f;
    s_gyro_sum[2] = 0.0f;
    s_cal_count = 0u;
    s_cal_target = INS_MANUAL_CALI_SAMPLES;
    s_status = INS_STATUS_MANUAL_CALI;
    s_attitude_seeded = 0;
#if INS_ENABLE_CONSOLE
    s_test_active = 0;
#endif
    taskEXIT_CRITICAL();
}

/* ---------------- 姿态解算 ---------------- */

static void ins_attitude_update(float gx, float gy, float gz,
                                float ax, float ay, float az)
{
    float accel_norm = sqrtf(ax * ax + ay * ay + az * az);

    if (!s_attitude_seeded && isfinite(accel_norm) &&
        accel_norm > INS_ACCEL_NORM_MIN && accel_norm < INS_ACCEL_NORM_MAX)
    {
        /* 上电第一次拿到可信加速度时，直接用加速度确定 roll/pitch，省掉收敛过程。 */
        s_accel_lpf[0] = ax;
        s_accel_lpf[1] = ay;
        s_accel_lpf[2] = az;

        float roll_acc  = atan2f(ay, az) * INS_RAD_TO_DEG;
        float pitch_acc = atan2f(-ax, sqrtf(ay * ay + az * az)) * INS_RAD_TO_DEG;
        ins_quat_from_roll_pitch(roll_acc, pitch_acc);
        s_yaw_offset = 0.0f;
        s_yaw_last = 0.0f;
        s_yaw_round = 0;
        s_attitude_seeded = 1;
    }

    if (isfinite(accel_norm) && accel_norm > INS_ACCEL_NORM_MIN &&
        accel_norm < INS_ACCEL_NORM_MAX)
    {
        s_accel_lpf[0] += INS_ACCEL_LPF_ALPHA * (ax - s_accel_lpf[0]);
        s_accel_lpf[1] += INS_ACCEL_LPF_ALPHA * (ay - s_accel_lpf[1]);
        s_accel_lpf[2] += INS_ACCEL_LPF_ALPHA * (az - s_accel_lpf[2]);

        float lx = s_accel_lpf[0];
        float ly = s_accel_lpf[1];
        float lz = s_accel_lpf[2];
        float ln = sqrtf(lx * lx + ly * ly + lz * lz);

        if (ln > 0.1f)
        {
            lx /= ln;
            ly /= ln;
            lz /= ln;

            float q0 = s_q[0], q1 = s_q[1], q2 = s_q[2], q3 = s_q[3];
            float vx = 2.0f * (q1 * q3 - q0 * q2);
            float vy = 2.0f * (q0 * q1 + q2 * q3);
            float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

            /* 实测重力方向 × 估计重力方向，比例项把姿态往加速度方向拉。 */
            float ex = ly * vz - lz * vy;
            float ey = lz * vx - lx * vz;
            float ez = lx * vy - ly * vx;

            gx += INS_MAHONY_KP * ex;
            gy += INS_MAHONY_KP * ey;
            gz += INS_MAHONY_KP * ez;
        }
    }

    if (!isfinite(gx) || !isfinite(gy) || !isfinite(gz))
    {
        return;
    }

    float q0 = s_q[0], q1 = s_q[1], q2 = s_q[2], q3 = s_q[3];
    float half_dt = 0.5f * INS_DT;

    s_q[0] += (-q1 * gx - q2 * gy - q3 * gz) * half_dt;
    s_q[1] += ( q0 * gx + q2 * gz - q3 * gy) * half_dt;
    s_q[2] += ( q0 * gy - q1 * gz + q3 * gx) * half_dt;
    s_q[3] += ( q0 * gz + q1 * gy - q2 * gx) * half_dt;
    ins_quat_normalize(s_q);

    q0 = s_q[0];
    q1 = s_q[1];
    q2 = s_q[2];
    q3 = s_q[3];

    float roll  = atan2f(2.0f * (q0 * q1 + q2 * q3),
                         1.0f - 2.0f * (q1 * q1 + q2 * q2)) * INS_RAD_TO_DEG;
    float pitch_arg = 2.0f * (q0 * q2 - q3 * q1);
    if (pitch_arg > 1.0f)  pitch_arg = 1.0f;
    if (pitch_arg < -1.0f) pitch_arg = -1.0f;
    float pitch = asinf(pitch_arg) * INS_RAD_TO_DEG;
    float yaw   = atan2f(2.0f * (q0 * q3 + q1 * q2),
                         1.0f - 2.0f * (q2 * q2 + q3 * q3)) * INS_RAD_TO_DEG;

    float yaw_rel = ins_wrap_180(yaw - s_yaw_offset);

    if (yaw_rel - s_yaw_last > 180.0f)
    {
        s_yaw_round--;
    }
    else if (yaw_rel - s_yaw_last < -180.0f)
    {
        s_yaw_round++;
    }

    s_data.q[0] = q0;
    s_data.q[1] = q1;
    s_data.q[2] = q2;
    s_data.q[3] = q3;
    s_data.roll = roll;
    s_data.pitch = pitch;
    s_data.yaw = yaw_rel;
    s_data.yaw_total_angle = 360.0f * (float)s_yaw_round + yaw_rel;
    s_yaw_last = yaw_rel;
}

/* ---------------- 对外接口 ---------------- */

void ins_init(void)
{
    /* 必须在 MX_SPI2_Init() 和 MX_GPIO_Init() 之后调用。 */
    if (s_inited)
    {
        return;
    }

    memset(&s_data, 0, sizeof(s_data));
    memset(&s_pub, 0, sizeof(s_pub));
    s_data.q[0] = 1.0f;
    s_pub.q[0] = 1.0f;
    memset(s_gyro_sum, 0, sizeof(s_gyro_sum));
    memset(s_gyro_bias, 0, sizeof(s_gyro_bias));
    ins_reset_attitude();

    s_error_code = 0u;
    s_inited = 1u;
    s_status = INS_STATUS_ERROR;

    uint8_t err = BMI088_init();
    if (err != BMI088_NO_ERROR)
    {
        s_error_code = err;
#if INS_ENABLE_CONSOLE
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "ins: BMI088 init failed, err=0x%02X\r\n",
                     (unsigned)err);
            ins_send(buf);
        }
#endif
        return;
    }

    s_cal_count = 0u;
    s_cal_target = INS_AUTO_CALI_SAMPLES;
    s_status = INS_STATUS_CALIBRATING;

#if INS_ENABLE_CONSOLE
    ins_send("ins: BMI088 ok, auto cali 1500 samples, keep still\r\n");
#endif
}

void ins_task_start(void)
{
    if (s_task == NULL)
    {
        s_task = osThreadNew(ins_task, NULL, &s_task_attr);
    }
}

void ins_task(void *argument)
{
    (void)argument;

    uint32_t wake = osKernelGetTickCount();
#if INS_ENABLE_CONSOLE
    uint32_t console_div = 0u;
    uint32_t uart_div = 0u;
#endif

    for (;;)
    {
        if (!s_inited)
        {
            osDelay(10);
            wake = osKernelGetTickCount();
            continue;
        }

        if (s_status == INS_STATUS_ERROR)
        {
            osDelay(100);
            wake = osKernelGetTickCount();
            continue;
        }

        imu_data raw;
        imu_read(&raw);
        memcpy(&s_data.raw, &raw, sizeof(raw));

        if (s_status == INS_STATUS_CALIBRATING || s_status == INS_STATUS_MANUAL_CALI)
        {
            s_gyro_sum[0] += raw.gyro[0];
            s_gyro_sum[1] += raw.gyro[1];
            s_gyro_sum[2] += raw.gyro[2];
            s_cal_count++;

            if (s_cal_count >= s_cal_target)
            {
                ins_finish_calibration();
            }
        }
        else if (s_status == INS_STATUS_READY)
        {
            float gx = raw.gyro[0] - s_gyro_bias[0];
            float gy = raw.gyro[1] - s_gyro_bias[1];
            float gz = raw.gyro[2] - s_gyro_bias[2];

            ins_attitude_update(gx, gy, gz,
                                raw.accel[0], raw.accel[1], raw.accel[2]);
        }

        /* 发布一份一致快照，别的任务读 s_pub 不会读到半更新数据。 */
        taskENTER_CRITICAL();
        s_pub = s_data;
        taskEXIT_CRITICAL();

#if INS_ENABLE_CONSOLE
        if (++console_div >= (INS_CONSOLE_PERIOD_MS / INS_TASK_PERIOD_MS))
        {
            console_div = 0u;
            ins_console_poll();
        }

        if (++uart_div >= (INS_UART_PERIOD_MS / INS_TASK_PERIOD_MS))
        {
            uart_div = 0u;
            if (s_watch)
            {
                ins_send_status_line();
            }
        }

        ins_test_poll();
#endif

        wake += INS_TASK_PERIOD_MS;
        osDelayUntil(wake);
    }
}

uint8_t ins_status(void)
{
    return s_status;
}

uint8_t ins_error_code(void)
{
    return s_error_code;
}

void ins_get_data(ins_data_t *out)
{
    if (out == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    *out = s_pub;
    taskEXIT_CRITICAL();
}

const ins_data_t *ins_data(void)
{
    return &s_pub;
}

void ins_zero_yaw(void)
{
    if (s_status != INS_STATUS_READY)
    {
        return;
    }

    taskENTER_CRITICAL();
#if INS_ENABLE_CONSOLE
    s_test_active = 0;
#endif
    s_yaw_offset += s_data.yaw;
    s_yaw_last = 0.0f;
    s_yaw_round = 0;
    s_data.yaw = 0.0f;
    s_data.yaw_total_angle = 0.0f;
    taskEXIT_CRITICAL();
}
