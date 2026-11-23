
#include "motor_ctrl.h"

#include "cmsis_os2.h"
#include "components/motor/dji/dji_motor.h"
#include "components/motor/dm/dm_motor.h"

void Startmotor_ctrl(void *argument)
{
    /* USER CODE BEGIN Startmotor_ctrl */
    /* Infinite loop */

    // dji_motor dji_motor1;
    // char name1[] = "dji_motor1";
    // dji_motor_create(&dji_motor1 , name1 , DJI_MODEL_M3508 , 2 , can1 , DJI_CTRL_CURRENT);
    // dji_motor_init(&dji_motor1);

    // dm_motor dm_mmotor1;
    // char name2[] = "dm_motor1";
    // dm_motor_create(&dm_mmotor1 , name2 , DM_MODEL_J4310 , 0x87 , 0x22 , can1 , DM_CTRL_MIT , 12.5 , 30 , 10);
    // dm_motor_init(&dm_mmotor1);
    // dm_motor_enable(&dm_mmotor1);

    for(;;)
    {
        // dji_motor_update(&dji_motor1 , 1000);
        // dji_motor_send();

        // dm_motor_control_mit(&dm_mmotor1 , 100 , 1000 , 0 , 0 , 2);

        osDelay(1);
    }
    /* USER CODE END Startmotor_ctrl */
}
