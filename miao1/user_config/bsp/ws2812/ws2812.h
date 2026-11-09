
#ifndef MIAO1_WS2812_H
#define MIAO1_WS2812_H

#include "main.h"
#include <stdint.h>
#include "stm32h7xx_hal_spi.h"

#define WS2812_SPI_UNIT     hspi6
extern SPI_HandleTypeDef WS2812_SPI_UNIT;
#define LED_BRIGHT_MAX 100

typedef enum
{
    blue    = 0x0000FF,
    green   = 0x00FF00,
    red     = 0xFF0000,
    white   = 0xFFFFFF,
    black   = 0x000000,
    yellow  = 0xFFFF00,
    cyan    = 0x00FFFF,
    orange  = 0xFF5800,
    grey    = 0x808080,
    gold    = 0xFFD700,
    purple  = 0x800080,
} led_color;

void WS2812_Ctrl(uint8_t r, uint8_t g, uint8_t b);
void led_control(led_color color,uint8_t bright);

#endif //MIAO1_WS2812_H
