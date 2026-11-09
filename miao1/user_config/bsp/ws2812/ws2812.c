#include "ws2812.h"

#define WS2812_LowLevel    0xC0
#define WS2812_HighLevel   0xF0

void WS2812_Ctrl(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t txbuf[24];
    uint8_t res = 0;
    for (int i = 0; i < 8; i++)
    {
        txbuf[7-i]  = (((g>>i)&0x01) ? WS2812_HighLevel : WS2812_LowLevel)>>1;
        txbuf[15-i] = (((r>>i)&0x01) ? WS2812_HighLevel : WS2812_LowLevel)>>1;
        txbuf[23-i] = (((b>>i)&0x01) ? WS2812_HighLevel : WS2812_LowLevel)>>1;
    }
    HAL_SPI_Transmit(&WS2812_SPI_UNIT, &res, 0, 0xFFFF);
    while (WS2812_SPI_UNIT.State != HAL_SPI_STATE_READY);
    HAL_SPI_Transmit(&WS2812_SPI_UNIT, txbuf, 24, 0xFFFF);
    for (int i = 0; i < 100; i++)
    {
        HAL_SPI_Transmit(&WS2812_SPI_UNIT, &res, 1, 0xFFFF);
    }
}

void  led_control(led_color color,uint8_t bright)
{
    int r,g,b;

    r = (((uint32_t)color >> 16) & 0xFF) * bright / LED_BRIGHT_MAX;
    g = (((uint32_t)color >> 8)  & 0xFF) * bright / LED_BRIGHT_MAX;
    b = ( (uint32_t)color        & 0xFF) * bright / LED_BRIGHT_MAX;

    if(r>255) r=255;
    if(g>255) g=255;
    if(b>255) b=255;

    WS2812_Ctrl(r,g,b);
}