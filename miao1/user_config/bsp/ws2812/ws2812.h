
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

/* ---------------- 彩色呼吸灯 ---------------- */
/* 呼吸节奏：一次“暗 -> 亮 -> 暗”的时长，单位 ms */
#define WS2812_BREATH_PERIOD_MS   2000
/* 色相绕一圈（红->黄->绿->青->蓝->品红->红）的时长，单位 ms */
#define WS2812_BREATH_HUE_MS      10000
/* 调用周期，单位 ms。任务里 osDelay 多少就填多少，节奏按它换算 */
#define WS2812_BREATH_TICK_MS     20

/* 彩色呼吸：亮度按正弦曲线起伏，颜色同时沿色相轮盘缓慢转动 */
void WS2812_Breath_Rainbow(uint8_t bright);
/* 单色呼吸：固定颜色，只有亮度起伏（可替代原来的 led_breath） */
void WS2812_Breath_Color(led_color color, uint8_t bright);
/* 相位归零：让呼吸从最暗处重新开始（可选，不调用也行） */
void WS2812_Breath_Reset(void);

#endif //MIAO1_WS2812_H
