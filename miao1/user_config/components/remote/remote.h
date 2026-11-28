
#ifndef MIAO1_REMOTE_HT_10A_H
#define MIAO1_REMOTE_HT_10A_H
#include <stdbool.h>
#include <stdint.h>

#define REMOTE_USE_UART uart5

typedef enum
{
    NOT_INIT ,
    HT_10A ,
    DT_7 ,
} remote_type ;

typedef struct
{
    bool status ;
    uint16_t CH1 ;
    uint16_t CH2 ;
    uint16_t CH3 ;
    uint16_t CH4 ;
    uint16_t CH5 ;
    uint16_t CH6 ;
    uint16_t CH7 ;
    uint16_t CH8 ;
    uint16_t CH9 ;
    uint16_t CH10 ;
    uint16_t CH11 ;
    uint16_t CH12 ;
    uint16_t CH13 ;
    uint16_t CH14 ;
    uint16_t CH15 ;
    uint16_t CH16 ;
} HT_10A_feedback_prt;

typedef struct
{
    bool status ;
    uint16_t r_x ;
    uint16_t r_y ;
    uint16_t l_x ;
    uint16_t l_y ;
    int swa ;
    int swb ;
    int swc ;
    int swd ;
    uint16_t vra ;
    uint16_t vrb ;
    uint32_t time ;
    uint32_t last_time ;
} HT_10A_feedback;

typedef struct
{
    bool status ;
    uint16_t CH1 ;
    uint16_t CH2 ;
    uint16_t CH3 ;
    uint16_t CH4 ;
    uint16_t CH5 ;
    uint16_t CH6 ;
    uint16_t CH7 ;
    uint16_t CH8 ;
    uint16_t CH9 ;
    uint16_t CH10 ;
    uint16_t CH11 ;
    uint16_t CH12 ;
    uint16_t CH13 ;
} DT_7_feedback_prt;

typedef struct
{
    uint16_t r_x ;
    uint16_t r_y ;
    uint16_t l_x ;
    uint16_t l_y ;
    uint8_t swa ;
    uint8_t swb ;
    uint16_t mouse_x ;
    uint16_t mouse_y ;
    uint16_t mouse_z ;
    uint8_t mouse_l ;
    uint8_t mouse_r ;
    uint16_t reserved ;
    uint32_t time ;
    uint32_t last_time ;
} DT_7_feedback;

void remote_type_switch(remote_type type);
remote_type receive_remote_type(void);
HT_10A_feedback_prt ht_10a_feedback_prt_send(void);
HT_10A_feedback ht_10a_feedback_send(void);
DT_7_feedback_prt dt_7_feedback_prt_send(void);

#endif //MIAO1_REMOTE_HT_10A_H
