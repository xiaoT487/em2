
#include "communication.h"

#include <string.h>

#include "cmsis_os2.h"
#include "bsp/can/user_can.h"
#include "bsp/usart/uart.h"
#include "freeRTOS/task_status/task_status.h"

void StartCommunication(void *argument)
{
    /* USER CODE BEGIN StartCommunication */
    /* 放到循环外面：放里面每轮都被清零，发出去的数据永远是同一个值 */
    uint8_t tx[8] = {0};
    static volatile uint32_t tx_ok;      /* 调试器里能直接看到 */
    static volatile uint32_t tx_err;

    /* Infinite loop */
    for(;;)
    {
        // tx[0]++;                                  /* 每次变化一下好认 */
        // if (can_send(can1, 0x101, tx, 8) == OK)   /* 成功是 OK(-1) */
        // {
        //     tx_ok++;
        // }
        // else
        // {
        //     tx_err++;                             /* 队列满 / 外设没 Start / 长度非法 */
        // }
        // osDelay(100);
    }
    /* USER CODE END StartCommunication */
}
