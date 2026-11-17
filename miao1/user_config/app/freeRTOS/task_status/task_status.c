#include "task_status.h"
#include "main.h"
#include "cmsis_os2.h"
#include "ws2812.h"
#include "bsp/buzzer/buzzer.h"

static void task_alarm(void)
{
    for (int i = 0; i < 3; i++)
    {
        led_control(red,   100);
        buzzer_alarm(30,100);
        osDelay(150);
        led_control(black,   100);
        buzzer_alarm(0,1000);
        osDelay(250);
    }

}

static void task_proceed(void)
{
    led_control(blue,   100);
    buzzer_alarm(30,100);
    osDelay(250);
    buzzer_alarm(0,1000);
}

static void task_normal(void)
{
    led_control(green,   100);
    buzzer_alarm(30,80);
    osDelay(150);
    buzzer_alarm(0,1000);
}

void Starttask_status(void *argument)
{
    /* USER CODE BEGIN Starttask_status */
    /* Infinite loop */
    for(;;)
    {
        led_control(blue,  100);
        osDelay(300);

        led_control(red,   100);
        osDelay(250);

        led_control(green, 100);
        osDelay(280);

        led_control(black, 100);
        osDelay(1000);

        task_proceed();
        osDelay(3000);
        task_alarm();
        osDelay(1000);

        task_proceed();
        osDelay(3000);
        task_normal();
        osDelay(1000);

    }
    /* USER CODE END Starttask_status */
}
