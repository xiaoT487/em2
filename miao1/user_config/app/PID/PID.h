
#ifndef MIAO1_PID_H
#define MIAO1_PID_H

typedef enum
{
    MOTOR1 ,
    MOTOR2 ,
    MOTOR3 ,
    MOTOR_NUM ,
} motor_type;

typedef struct
{
    double target;
    double current;
    double error;
    double last_error;
    double integral;
    double derivative;
    double kp;
    double ki;
    double kd;
    double integral_max;
    double integral_min;
    double derivative_max;
    double derivative_min;
    double sampling_time;
    double max_output;
} pid_type;

extern pid_type speed_pid[MOTOR_NUM];
extern pid_type angle_pid[MOTOR_NUM];
extern pid_type current_pid[MOTOR_NUM];

void speed_filtering_init(motor_type device, double SPEED_LPF_ALPHA_input);
double speed_filtering(motor_type device , double speed_input);
void PID_Init(pid_type *pid ,double kp, double ki, double kd, double integral_max,
              double integral_min, double derivative_max, double derivative_min ,
              double sampling_time , double max_output);
double PID_CALCULATE(pid_type *pid,double target,double current);

#endif //MIAO1_PID_H
