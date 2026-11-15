
#ifndef MIAO1_IMU_H
#define MIAO1_IMU_H

typedef struct {
    float gyro[3], accel[3], temp;
} imu_data;

void imu_read(imu_data *imu_data);

#endif //MIAO1_IMU_H
