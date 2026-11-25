#include "BMI088Middleware.h"
#include "main.h"

#define BMI088_USING_SPI_UNIT   hspi2

extern SPI_HandleTypeDef BMI088_USING_SPI_UNIT;

/**
************************************************************************
* @brief:      	BMI088_GPIO_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088传感器GPIO初始化函�?************************************************************************
**/
void BMI088_GPIO_init(void)
{
    /* 上电先把两路片选都拉高，避免两个 BMI088 die 同时挂在 MISO 上。 */
    HAL_GPIO_WritePin(SPI2_CS0_GPIO_Port, SPI2_CS0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SPI2_CS1_GPIO_Port, SPI2_CS1_Pin, GPIO_PIN_SET);
}
/**
************************************************************************
* @brief:      	BMI088_com_init(void)
* @param:       void
* @retval:     	void
* @details:    	BMI088传感器通信初始化函�?************************************************************************
**/
void BMI088_com_init(void)
{


}
/**
************************************************************************
* @brief:      	BMI088_delay_ms(uint16_t ms)
* @param:       ms - 要延迟的毫秒�?* @retval:     	void
* @details:    	延迟指定毫秒数的函数，基于微秒延迟实�?************************************************************************
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
* @param:       us - 要延迟的微秒�?* @retval:     	void
* @details:    	微秒级延迟函数，使用SysTick定时器实�?************************************************************************
**/
void BMI088_delay_us(uint16_t us)
{
    /* ԭ����д���� SysTick->VAL �������������� HAL ʱ���� TIM1��
       SysTick Ҫ�� osKernelStart() ֮����ܣ�BMI088_init ����֮ǰ���ã���
       VAL ��Ϊ 0 => tnow == told ��Զ���� => while(1) ���ȣ�����������
       ������� DWT �ں����ڼ����������κζ�ʱ��/ʱ�Ӷ��޹أ������Ῠ�� */
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
* @details:    	将BMI088加速度计片选信号置低，使其处于选中状�?************************************************************************
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
* @details:    	将BMI088加速度计片选信号置高，使其处于非选中状�?************************************************************************
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
* @details:    	将BMI088陀螺仪片选信号置低，使其处于选中状�?************************************************************************
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
* @details:    	将BMI088陀螺仪片选信号置高，使其处于非选中状�?************************************************************************
**/
void BMI088_GYRO_NS_H(void)
{
    HAL_GPIO_WritePin(SPI2_CS1_GPIO_Port, SPI2_CS1_Pin, GPIO_PIN_SET);
}
/**
************************************************************************
* @brief:      	BMI088_read_write_byte(uint8_t txdata)
* @param:       txdata - 要发送的数据
* @retval:     	uint8_t - 接收到的数据
* @details:    	通过BMI088使用的SPI总线进行单字节的读写操作
************************************************************************
**/
uint8_t BMI088_read_write_byte(uint8_t txdata)
{
    uint8_t rx_data = 0;

    /* 正常一字节只要几微秒；超时设 1 ms，总线坏了也不会把 1 kHz 任务卡住 10 ms。 */
    if (HAL_SPI_TransmitReceive(&BMI088_USING_SPI_UNIT, &txdata, &rx_data, 1, 1) != HAL_OK)
    {
        return 0;
    }

    return rx_data;
}

