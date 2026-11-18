#include "task_status.h"

#include <string.h>

#include "main.h"
#include "cmsis_os2.h"
#include "ws2812.h"
#include "bsp/buzzer/buzzer.h"
#include "bsp/usart/uart.h"

volatile int task_link = 0 ;
volatile int task_busy = 0 ;
volatile int task_link_status = 0 ;

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

void task_start(void)
{
    task_busy=1;
    task_link=1;
}

void task_end(task_result task_result)
{
    if (task_result == TASK_OK)
    {
        task_busy=0;
        task_link=0;
        task_link_status=2;
    }

    if (task_result == TASK_ERROR)
    {
        task_busy=0;
        task_link=0;
        task_link_status=1;
    }
}


void Starttask_status(void *argument)
{
    /* USER CODE BEGIN Starttask_status */
    /* Infinite loop */
    for(;;)
    {

        if (task_busy == 0)
        {
            WS2812_Breath_Rainbow(100);
            osDelay(20);
        } else if (task_busy == 1)
        {
            osDelay(20);
            if (task_link == 1)
            {
                task_proceed();
                while (task_link == 1)
                {
                    osDelay(20);
                }
                if (task_link_status == 1)
                    task_alarm();
                if (task_link_status == 2)
                    task_normal();
                osDelay(20);
            }
        }

    }
    /* USER CODE END Starttask_status */
}
