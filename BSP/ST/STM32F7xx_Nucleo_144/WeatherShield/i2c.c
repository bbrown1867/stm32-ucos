/**
 * @file   i2c.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  I2C shims for the MS8607 driver.
 */

#include "i2c.h"

#include <os.h>
#include <stm32f7xx.h>

#include <stdint.h>
#include <stdlib.h>

#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
#define I2Cx_SCL_PIN                    GPIO_PIN_8
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_GPIO_AF                GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_9
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_GPIO_AF                GPIO_AF4_I2C1

#define I2C_TRANSFER_TIMEOUT_TICKS      (1000U)

static I2C_HandleTypeDef I2cHandle;
static I2C_HandleTypeDef* pI2cHandle = NULL;

static enum status_code HalStatus2DriverStatus(HAL_StatusTypeDef status)
{
    if (status == HAL_OK)
    {
        return STATUS_OK;
    }
    else if (status == HAL_TIMEOUT)
    {
        return STATUS_ERR_TIMEOUT;
    }
    else
    {
        return STATUS_ERR_OVERFLOW;
    }
}

void delay_ms(uint32_t duration_ms)
{
    /* Ignore errors */
    OS_ERR err;

    OSTimeDlyHMSM((CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT16U) duration_ms,
                  (OS_OPT)     OS_OPT_TIME_HMSM_NON_STRICT | OS_OPT_TIME_DLY,
                  (OS_ERR*)    &err);
}

void i2c_master_init(void)
{
    if (pI2cHandle == NULL)
    {
        pI2cHandle = &I2cHandle;

        /* I2C timing: 0 ns rise and fall time, 100 KHz SCL */
        pI2cHandle->Instance              = I2Cx;
        pI2cHandle->Init.Timing           = 0x20303E5D;
        pI2cHandle->Init.OwnAddress1      = 0x00;
        pI2cHandle->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
        pI2cHandle->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
        pI2cHandle->Init.OwnAddress2      = 0x00;
        pI2cHandle->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
        pI2cHandle->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
        pI2cHandle->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

        (void) HAL_I2C_Init(&I2cHandle);
    }
}

enum status_code i2c_master_read_packet_wait(struct i2c_master_packet *const packet)
{
    HAL_StatusTypeDef result;

    result = HAL_I2C_Master_Receive(pI2cHandle,
                                    (packet->address << 1) | 0x01,
                                    packet->data,
                                    packet->data_length,
                                    I2C_TRANSFER_TIMEOUT_TICKS);

    return HalStatus2DriverStatus(result);
}

enum status_code i2c_master_write_packet_wait(struct i2c_master_packet *const packet)
{
    HAL_StatusTypeDef result;

    result = HAL_I2C_Master_Transmit(pI2cHandle,
                                     (packet->address << 1),
                                     packet->data,
                                     packet->data_length,
                                     I2C_TRANSFER_TIMEOUT_TICKS);

    return HalStatus2DriverStatus(result);
}
enum status_code i2c_master_write_packet_wait_no_stop(struct i2c_master_packet *const packet)
{
    HAL_StatusTypeDef result;

    result = HAL_I2C_Master_Transmit(pI2cHandle,
                                     (packet->address << 1),
                                     packet->data,
                                     packet->data_length,
                                     I2C_TRANSFER_TIMEOUT_TICKS);

    return HalStatus2DriverStatus(result);
}

/*
 * STM32 HAL functions.
 */

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable I2C clock */
    I2Cx_CLK_ENABLE();

    /* Enable I2C GPIO clocks */
    I2Cx_SCL_GPIO_CLK_ENABLE();
    I2Cx_SDA_GPIO_CLK_ENABLE();

    /* Common I2C GPIO pin configuration */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

    /* SCL I2C GPIO pin configuration */
    GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
    GPIO_InitStruct.Alternate = I2Cx_SCL_GPIO_AF;
    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* SDA I2C GPIO pin configuration */
    GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
    GPIO_InitStruct.Alternate = I2Cx_SDA_GPIO_AF;
    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    /* Reset I2C clock */
    I2Cx_FORCE_RESET();
    I2Cx_RELEASE_RESET();

    /* Reset I2C GPIO pin configurations */
    HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
    HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);
}
