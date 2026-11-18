
#include "PID.h"

#include <stdint.h>

pid_type speed_pid[MOTOR_NUM];
pid_type angle_pid[MOTOR_NUM];
pid_type current_pid[MOTOR_NUM];

static double SPEED_LPF_ALPHA[MOTOR_NUM];

static uint8_t speed_lpf_first[MOTOR_NUM] = {1};
static double speed_lpf_prev[MOTOR_NUM];

void speed_filtering_init(motor_type device, double SPEED_LPF_ALPHA_input)
{
    SPEED_LPF_ALPHA[device]=SPEED_LPF_ALPHA_input;
}

double speed_filtering(motor_type device , double speed_input)
{
    if (speed_lpf_first[device] == 1)
    {
        speed_lpf_prev[device] = speed_input;
        speed_lpf_first[device]=0;
    } else speed_lpf_prev[device] += SPEED_LPF_ALPHA[device] * (speed_input - speed_lpf_prev[device]);
    return speed_lpf_prev[device];
}

void PID_Init(pid_type *pid ,double kp, double ki, double kd, double integral_max,
              double integral_min, double derivative_max, double derivative_min ,
              double sampling_time , double max_output)
{
    pid->kp=kp;
    pid->ki=ki;
    pid->kd=kd;
    pid->integral_max=integral_max;
    pid->integral_min=integral_min;
    pid->derivative_max=derivative_max;
    pid->derivative_min=derivative_min;
    pid->error=0;
    pid->last_error=0;
    pid->integral=0;
    pid->derivative=0;
    pid->sampling_time=sampling_time;
    pid->max_output=max_output;
}

double PID_CALCULATE(pid_type *pid,double target,double current)
{
    pid->target=target;
    pid->current=current;

    pid->error=pid->target-pid->current;

    pid->integral=(pid->error*pid->sampling_time)+pid->integral;
    if (pid->integral>pid->integral_max)
    {
        pid->integral=pid->integral_max;
    }
    if (pid->integral<pid->integral_min)
    {
        pid->integral=pid->integral_min;
    }

    pid->derivative=(pid->error-pid->last_error)/pid->sampling_time;
    if (pid->derivative>pid->derivative_max)
    {
        pid->derivative=pid->derivative_max;
    }
    if (pid->derivative<pid->derivative_min)
    {
        pid->derivative=pid->derivative_min;
    }

    double output;
    output=pid->kp*pid->error+pid->ki*pid->integral+pid->kd*pid->derivative;

    if (output>pid->max_output)
    {
        output=pid->max_output;
    }
    if (output<-(pid->max_output))
    {
        output=-(pid->max_output);
    }
    return output;
}