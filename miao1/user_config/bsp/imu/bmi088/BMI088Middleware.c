#include "BMI088Middleware.h"
#include "main.h"

#define BMI088_USING_SPI_UNIT   hspi2

extern SPI_HandleTypeDef BMI088_USING_SPI_UNIT;

/**
************************************************************************
* @brief:      	BMI088_GPIO_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088ä¼ æ„Ÿå™¨GPIOåˆå§‹åŒ–å‡½æ•?************************************************************************
**/
void BMI088_GPIO_init(void)
{

}
/**
************************************************************************
* @brief:      	BMI088_com_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088ä¼ æ„Ÿå™¨é€šä¿¡åˆå§‹åŒ–å‡½æ•?************************************************************************
**/
void BMI088_com_init(void)
{


}
/**
************************************************************************
* @brief:      	BMI088_delay_ms(uint16_t ms)
* @param:       ms - è¦å»¶è¿Ÿçš„æ¯«ç§’æ•?* @retval:     	void
* @details:    	å»¶è¿ŸæŒ‡å®šæ¯«ç§’æ•°çš„å‡½æ•°ï¼ŒåŸºäºŽå¾®ç§’å»¶è¿Ÿå®žçŽ?************************************************************************
**/
void BMI088_delay_ms(uint16_t ms)
{
    while(ms--)
    {
        BMI088_delay_us(1000);
    }
}
/**
************************************************************************
* @brief:      	BMI088_delay_us(uint16_t us)
* @param:       us - è¦å»¶è¿Ÿçš„å¾®ç§’æ•?* @retval:     	void
* @details:    	å¾®ç§’çº§å»¶è¿Ÿå‡½æ•°ï¼Œä½¿ç”¨SysTickå®šæ—¶å™¨å®žçŽ?************************************************************************
**/
void BMI088_delay_us(uint16_t us)
{
    /* Ô­À´µÄÐ´·¨ÓÃ SysTick->VAL ¼ÆÊý£¬µ«±¾¹¤³Ì HAL Ê±»ùÊÇ TIM1£¬
       SysTick ÒªµÈ osKernelStart() Ö®ºó²ÅÅÜ£¨BMI088_init ÔÚËüÖ®Ç°µ÷ÓÃ£©£¬
       VAL ºãÎª 0 => tnow == told ÓÀÔ¶³ÉÁ¢ => while(1) ËÀµÈ£¬Õû»ú¿¨ËÀ¡£
       ÕâÀï¸ÄÓÃ DWT ÄÚºËÖÜÆÚ¼ÆÊýÆ÷£¬¸úÈÎºÎ¶¨Ê±Æ÷/Ê±ÖÓ¶¼ÎÞ¹Ø£¬¾ø²»»á¿¨¡£ */
    static uint8_t dwt_ready = 0;
    if (!dwt_ready)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        dwt_ready = 1;
    }
    if (us == 0)
    {
        return;
    }
    uint32_t cycles = (uint32_t)us * (SystemCoreClock / 1000000U);
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles)
    {
    }
}
/**
************************************************************************
* @brief:      	BMI088_ACCEL_NS_L(void)
* @param:       void
* @retval:     	void
* @details:    	å°†BMI088åŠ é€Ÿåº¦è®¡ç‰‡é€‰ä¿¡å·ç½®ä½Žï¼Œä½¿å…¶å¤„äºŽé€‰ä¸­çŠ¶æ€?************************************************************************
**/
void BMI088_ACCEL_NS_L(void)
{
    HAL_GPIO_WritePin(SPI2_CS0_GPIO_Port, SPI2_CS0_Pin, GPIO_PIN_RESET);
}
/**
************************************************************************
* @brief:      	BMI088_ACCEL_NS_H(void)
* @param:       void
* @retval:     	void
* @details:    	å°†BMI088åŠ é€Ÿåº¦è®¡ç‰‡é€‰ä¿¡å·ç½®é«˜ï¼Œä½¿å…¶å¤„äºŽéžé€‰ä¸­çŠ¶æ€?************************************************************************
**/
void BMI088_ACCEL_NS_H(void)
{
    HAL_GPIO_WritePin(SPI2_CS0_GPIO_Port, SPI2_CS0_Pin, GPIO_PIN_SET);
}
/**
************************************************************************
* @brief:      	BMI088_GYRO_NS_L(void)
* @param:       void
* @retval:     	void
* @details:    	å°†BMI088é™€èžºä»ªç‰‡é€‰ä¿¡å·ç½®ä½Žï¼Œä½¿å…¶å¤„äºŽé€‰ä¸­çŠ¶æ€?************************************************************************
**/
void BMI088_GYRO_NS_L(void)
{
    HAL_GPIO_WritePin(SPI2_CS1_GPIO_Port, SPI2_CS1_Pin, GPIO_PIN_RESET);
}
/**
************************************************************************
* @brief:      	BMI088_GYRO_NS_H(void)
* @param:       void
* @retval:     	void
* @details:    	å°†BMI088é™€èžºä»ªç‰‡é€‰ä¿¡å·ç½®é«˜ï¼Œä½¿å…¶å¤„äºŽéžé€‰ä¸­çŠ¶æ€?************************************************************************
**/
void BMI088_GYRO_NS_H(void)
{
    HAL_GPIO_WritePin(SPI2_CS1_GPIO_Port, SPI2_CS1_Pin, GPIO_PIN_SET);
}
/**
************************************************************************
* @brief:      	BMI088_read_write_byte(uint8_t txdata)
* @param:       txdata - è¦å‘é€çš„æ•°æ®
* @retval:     	uint8_t - æŽ¥æ”¶åˆ°çš„æ•°æ®
* @details:    	é€šè¿‡BMI088ä½¿ç”¨çš„SPIæ€»çº¿è¿›è¡Œå•å­—èŠ‚çš„è¯»å†™æ“ä½œ
************************************************************************
**/
uint8_t BMI088_read_write_byte(uint8_t txdata)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&BMI088_USING_SPI_UNIT, &txdata, &rx_data, 1, 10);
    return rx_data;
}

