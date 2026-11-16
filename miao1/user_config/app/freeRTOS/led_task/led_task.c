#include "led_task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "ws2812.h"



void Startled_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        led_control(blue,  100);
        osDelay(300);

        led_control(red,   100);
        osDelay(250);

        led_control(green, 100);
        osDelay(280);

        led_control(black, 100);
        osDelay(1000);
    }
}
