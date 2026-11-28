
#include "communication.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "bsp/can/user_can.h"
#include "bsp/usart/uart.h"
#include "../../../components/remote/remote.h"
#include "freeRTOS/task_status/task_status.h"

void StartCommunication(void *argument)
{
    /* USER CODE BEGIN StartCommunication */
    static uint8_t buff[128];
    HT_10A_feedback j;
    DT_7_feedback_prt k;

    remote_type_switch(DT_7);

    /* Infinite loop */
    for(;;)
    {
        if (receive_remote_type() == HT_10A)
        {
            j = ht_10a_feedback_send();

            (void)snprintf(buff, sizeof buff,
                    "%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d\r\n",
                    j.r_x,j.r_y,j.l_x,j.l_y,j.swa,j.swb,j.swc,j.swd,j.vra,j.vrb,j.time);

            UART_send_async(uart1, buff, (uint16_t)strlen(buff));
        }

        if (receive_remote_type() == DT_7)
        {
            k = dt_7_feedback_prt_send();

            (void)snprintf(buff, sizeof buff,
                     "%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d,%4d\r\n",
                     k.CH1,k.CH2,k.CH3,k.CH4,k.CH5,k.CH6,k.CH7,k.CH8,k.CH9,k.CH10,k.CH11,k.CH12,k.CH13);

            UART_send_async(uart1, buff, (uint16_t)strlen(buff));
        }

        osDelay(50);
    }
    /* USER CODE END StartCommunication */
}
