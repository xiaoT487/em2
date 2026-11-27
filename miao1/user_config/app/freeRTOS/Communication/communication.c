
#include "communication.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "bsp/can/user_can.h"
#include "bsp/usart/uart.h"
#include "components/remote/HT-10A/remote_HT-10A.h"
#include "freeRTOS/task_status/task_status.h"

void StartCommunication(void *argument)
{
    /* USER CODE BEGIN StartCommunication */
    static uint8_t buff[128];
    HT_10A_feedback j;

    /* Infinite loop */
    for(;;)
    {
        j = ht_10a_feedback_send();

        (void)snprintf(buff, sizeof buff,
                "%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d\r\n",
                j.r_x,j.r_y,j.l_x,j.l_y,j.swa,j.swb,j.swc,j.swd,j.vra,j.vrb,j.time);

        UART_send_async(uart1, buff, (uint16_t)strlen(buff));
        osDelay(50);
    }
    /* USER CODE END StartCommunication */
}
