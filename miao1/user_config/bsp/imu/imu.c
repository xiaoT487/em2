
#include "imu.h"
#include "bmi088/BMI088driver.h"

void imu_read(imu_data *imu_data)
{
    BMI088_read(imu_data->gyro, imu_data->accel, &imu_data->temp);
}
