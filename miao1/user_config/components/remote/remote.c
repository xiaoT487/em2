

#include "remote.h"

#include "cmsis_os2.h"
#include "uart.h"

static remote_type remote_mode = NOT_INIT ;

static  HT_10A_feedback_prt ht_10_a_feedback_prt;
static  HT_10A_feedback ht_10_a_feedback;

static DT_7_feedback_prt dt_7_feedback_prt;
static DT_7_feedback dt_7_feedback;


void remote_type_switch(remote_type type)
{

    if (type == NOT_INIT) return;

    remote_mode = type ;

    if (type == HT_10A )
    {
        huart5.Init.BaudRate   = 100000;
        huart5.Init.WordLength = UART_WORDLENGTH_9B;
        huart5.Init.Parity     = UART_PARITY_NONE;
        huart5.Init.StopBits   = UART_STOPBITS_2;
    }
    if (type == DT_7 )
    {
        huart5.Init.BaudRate   = 100000;
        huart5.Init.WordLength = UART_WORDLENGTH_8B;
        huart5.Init.Parity     = UART_PARITY_EVEN;
        huart5.Init.StopBits   = UART_STOPBITS_1;
    }
}

remote_type receive_remote_type(void)
{
    return remote_mode ;
}

static HT_10A_feedback_prt date_feedback( uint8_t *feedback )
{
    HT_10A_feedback_prt raw;
    raw.CH1 = (uint16_t)((feedback[0] | feedback[1] << 8) & 0x07FF) ;
    raw.CH2 = (uint16_t)((feedback[1] >>3 | feedback[2] << 5) & 0x07FF) ;
    raw.CH3 = (uint16_t)((feedback[2] >> 6 | feedback[3] << 2 | feedback[4] << 10) & 0x07FF) ;
    raw.CH4 = (uint16_t)((feedback[4] >> 1 | feedback[5] << 7) & 0x07FF) ;
    raw.CH5 = (uint16_t)((feedback[5] >> 4 | feedback[6] << 4) & 0x07FF) ;
    raw.CH6 = (uint16_t)((feedback[6] >> 7 | feedback[7] << 1 | feedback[8] << 9) & 0x07FF) ;
    raw.CH7 = (uint16_t)((feedback[8] >> 2 | feedback[9] << 6) & 0x07FF) ;
    raw.CH8 = (uint16_t)((feedback[9] >> 5 | feedback[10] << 3) & 0x07FF) ;
    raw.CH9 = (uint16_t)((feedback[11] | feedback[12] << 8) & 0x07FF) ;
    raw.CH10 = (uint16_t)((feedback[12] >> 3 | feedback[13] << 5) & 0x07FF) ;
    raw.CH11 = (uint16_t)((feedback[13] >> 6 | feedback[14] << 2 | feedback[15] << 10) & 0x07FF) ;
    raw.CH12 = (uint16_t)((feedback[15] >> 1 | feedback[16] << 7) & 0x07FF) ;
    raw.CH13 = (uint16_t)((feedback[16] >> 4 | feedback[17] << 4) & 0x07FF) ;
    raw.CH14 = (uint16_t)((feedback[17] >> 7 | feedback[18] << 1 | feedback[19] << 9) & 0x07FF) ;
    raw.CH15 = (uint16_t)((feedback[19] >> 2 | feedback[20] << 6) & 0x07FF) ;
    raw.CH16 =(uint16_t)((feedback[20] >> 5 | feedback[21] << 3) & 0x07FF) ;

    if (feedback[23] == 0x00 || feedback[23] == 0x04 || feedback[23] == 0x14 || feedback[23] == 0x24)
        raw.status = true ;
    else raw.status = false ;

    return raw ;
}

static HT_10A_feedback cul_feedback(HT_10A_feedback_prt raw)
{
    HT_10A_feedback cul;

    cul.r_x = raw.CH1;
    cul.r_y = raw.CH2;
    cul.l_x = raw.CH3;
    cul.l_y = raw.CH4;

    if (raw.CH5 == 192) cul.swa = -1;
    else if (raw.CH5 == 992) cul.swa = 0;
    else cul.swa = 1;

    if (raw.CH6 == 192) cul.swb = -1;
    else cul.swb = 1;

    if (raw.CH7 == 192) cul.swc = -1;
    else cul.swc = 1;

    if (raw.CH8 == 192) cul.swd = -1;
    else if (raw.CH8 == 992) cul.swd = 0;
    else cul.swd = 1;

    cul.vra = raw.CH9;
    cul.vrb = raw.CH10;

    cul.status = raw.status;

    return cul ;
}

HT_10A_feedback_prt ht_10a_feedback_prt_send(void)
{
    return ht_10_a_feedback_prt ;
}

HT_10A_feedback ht_10a_feedback_send(void)
{
    return ht_10_a_feedback ;
}


static DT_7_feedback_prt date_feedback2(uint8_t *feedback)
{
    DT_7_feedback_prt raw ;
    raw.CH1 = (uint16_t)((feedback[0] | feedback[1] << 8) & 0x07FF) ;
    raw.CH2 = (uint16_t)((feedback[1] >>3 | feedback[2] << 5) & 0x07FF) ;
    raw.CH3 = (uint16_t)((feedback[2] >> 6 | feedback[3] << 2 | feedback[4] << 10) & 0x07FF) ;
    raw.CH4 = (uint16_t)((feedback[4] >> 1 | feedback[5] << 7) & 0x07FF) ;
    raw.CH5 = (uint8_t)((feedback[5] >> 4) & 0x0003) ;
    raw.CH6 = (uint8_t)((feedback[5] >> 6) & 0x0003) ;
    raw.CH7 = (uint16_t)((feedback[6] | feedback[7] << 8) & 0xFFFF) ;
    raw.CH8 = (uint16_t)((feedback[8] | feedback[9] << 8) & 0xFFFF) ;
    raw.CH9 = (uint16_t)((feedback[10] | feedback[11] << 8) & 0xFFFF) ;
    raw.CH10 = (uint8_t)(feedback[12] & 0x00FF) ;
    raw.CH11 = (uint8_t)(feedback[13] & 0x00FF) ;
    raw.CH12 = (uint16_t)((feedback[14] | feedback[15] << 8) & 0xFFFF) ;
    raw.CH13 = (uint16_t)((feedback[16] | feedback[17] << 8) & 0xFFFF) ;

    if (raw.CH5 > 0 && raw.CH5 < 4 && raw.CH6 > 0 && raw.CH6 < 4) raw.status = true ;
    else raw.status = false ;

    return raw ;
}

DT_7_feedback_prt dt_7_feedback_prt_send(void)
{
    return dt_7_feedback_prt ;
}


void StartHT10A(void *argument)
{
    /* USER CODE BEGIN StartHT10A */
    /* Infinite loop */
    uint8_t date1 = 0;
    uint8_t date2[24] = {0};
    uint8_t date3[18] = {0};
    HT_10A_feedback_prt feedback_judge;
    DT_7_feedback_prt feedback_judge2;

    for(;;)
    {
        if (remote_mode == HT_10A)
        {
            Command_rx_ReadData(REMOTE_USE_UART , &date1 , 1);

            if (date1 == 0x0F)
            {
                Command_rx_ReadData(REMOTE_USE_UART , date2 , 24);
                feedback_judge = date_feedback(date2);
                if (feedback_judge.status == true)
                {
                    ht_10_a_feedback_prt = feedback_judge;
                    ht_10_a_feedback = cul_feedback(feedback_judge);
                    ht_10_a_feedback.last_time = ht_10_a_feedback.time;
                    ht_10_a_feedback.time = HAL_GetTick();
                }
            }
        }

        if (remote_mode == DT_7)
        {
            if (HAL_GetTick() - rx_time > 10 && rx_time != 0)
            {
                clear_rx_buffer(REMOTE_USE_UART);
                while (HAL_GetTick() - rx_time > 10) osDelay(1);
                Command_rx_ReadData(REMOTE_USE_UART , date3 , 18);
                dt_7_feedback_prt = date_feedback2(date3);

            }
        }
        osDelay(1);
    }
    /* USER CODE END StartHT10A */
}
