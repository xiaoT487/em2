
#include "buzzer.h"

#include "stm32h7xx_hal.h"
#include "tim.h"

void buzzer_alarm(int loudness ,int frequency)
{
    if (loudness>100)
    {
        loudness=100;
    } else if (loudness<0)
    {
        loudness=0;
    }

    if (frequency>10000)
    {
        frequency=10000;
    } else if (frequency<0)
    {
        frequency=0;
    }

    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, loudness* buzzer_loudness_max /100);
    __HAL_TIM_SET_AUTORELOAD(&htim12, frequency);

}
