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

/* ==================== 彩色呼吸灯 ==================== */
/*
 * 思路：把“亮度”和“颜色”拆成两个独立的相位。
 *   亮度相位 -> 查 sin² 曲线，得到 0~255 的亮度系数（两端变化率为 0，所以呼吸顺滑不突兀）
 *   色相相位 -> 绕 HSV 色轮转圈，得到颜色
 * 两个相位都用 uint16_t 累加，溢出自动回绕，不用管时间，也不用 sin() 浮点运算。
 *
 * 注意：WS2812_Ctrl 是阻塞发送，同一时刻只能有一个任务调用本组函数。
 */

/* sin² 呼吸曲线：64 点，0~255。两头暗、中间亮，且两端导数为 0，看起来最像“呼吸” */
static const uint8_t breath_lut[64] = {
      0,   1,   2,   5,  10,  15,  21,  29,
     37,  47,  57,  67,  79,  90, 103, 115,
    127, 140, 152, 165, 176, 188, 198, 208,
    218, 226, 234, 240, 245, 250, 253, 254,
    255, 254, 253, 250, 245, 240, 234, 226,
    218, 208, 198, 188, 176, 165, 152, 140,
    128, 115, 103,  90,  79,  67,  57,  47,
     37,  29,  21,  15,  10,   5,   2,   1,
};

static uint16_t s_breath_phase;   /* 呼吸相位 0~65535 */
static uint16_t s_hue_phase;      /* 色相相位 0~65535 */

/* 每个 tick 相位走多少：一整圈 = 65536，走完一圈的时长 = period_ms */
static uint16_t phase_step(uint32_t period_ms)
{
    uint32_t step;

    if (period_ms == 0)
    {
        period_ms = 1;
    }
    step = (65536UL * WS2812_BREATH_TICK_MS) / period_ms;

    if (step == 0)      /* 周期设得太长，至少也要动一下，否则灯是死的 */
    {
        step = 1;
    }
    if (step > 0xFFFF)  /* 周期设得太短，别让相位一步跨过一整圈 */
    {
        step = 0xFFFF;
    }

    return (uint16_t)step;
}

void WS2812_Breath_Reset(void)
{
    s_breath_phase = 0;
    s_hue_phase    = 0;
}

/* 推进呼吸相位，返回当前亮度 0 ~ bright（bright 按 LED_BRIGHT_MAX=100 的百分比算） */
static uint8_t breath_level(uint8_t bright)
{
    uint8_t curve;

    if (bright > LED_BRIGHT_MAX)
    {
        bright = LED_BRIGHT_MAX;
    }

    s_breath_phase += phase_step(WS2812_BREATH_PERIOD_MS);
    curve = breath_lut[s_breath_phase >> 10];       /* 高 6 位当索引，64 点 */

    return (uint8_t)((uint16_t)curve * bright / LED_BRIGHT_MAX);
}

void WS2812_Breath_Rainbow(uint8_t bright)
{
    uint8_t  v = breath_level(bright);
    uint16_t h;
    uint8_t  seg, pos, rising, falling;
    uint8_t  r, g, b;

    /* 色相 0~1535 分成 6 段，每段 256，刚好对应红->黄->绿->青->蓝->品红->红 */
    s_hue_phase += phase_step(WS2812_BREATH_HUE_MS);
    h   = (uint16_t)(((uint32_t)s_hue_phase * 1536u) >> 16);
    seg = (uint8_t)(h >> 8);          /* 第几段 0~5 */
    pos = (uint8_t)(h & 0xFF);        /* 段内位置 0~255 */

    rising  = (uint8_t)((uint16_t)v * pos / 255);          /* 往 v 爬升 */
    falling = (uint8_t)((uint16_t)v * (255u - pos) / 255); /* 从 v 回落 */

    switch (seg)
    {
        case 0:  r = v;       g = rising;  b = 0;       break;  /* 红 -> 黄 */
        case 1:  r = falling; g = v;       b = 0;       break;  /* 黄 -> 绿 */
        case 2:  r = 0;       g = v;       b = rising;  break;  /* 绿 -> 青 */
        case 3:  r = 0;       g = falling; b = v;       break;  /* 青 -> 蓝 */
        case 4:  r = rising;  g = 0;       b = v;       break;  /* 蓝 -> 品红 */
        default: r = v;       g = 0;       b = falling; break;  /* 品红 -> 红 */
    }

    WS2812_Ctrl(r, g, b);
}

void WS2812_Breath_Color(led_color color, uint8_t bright)
{
    uint8_t v = breath_level(bright);
    uint8_t r, g, b;

    r = (uint8_t)((((uint32_t)color >> 16) & 0xFF) * v / 255);
    g = (uint8_t)((((uint32_t)color >> 8)  & 0xFF) * v / 255);
    b = (uint8_t)(( (uint32_t)color        & 0xFF) * v / 255);

    WS2812_Ctrl(r, g, b);
}