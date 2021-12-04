/**
 * @file   bsp_sensor.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Thread-safe driver for TE Connectivity Weather Shield.
 *
 *         The Weather Shield contains 5 different environmental
 *         sensors which each measure some combination of temperature,
 *         humidity, and pressure. These sensors share a single I2C bus
 *         which is routed through a 4-1 mux to the microcontroller since
 *         some sensors share the same I2C address.
 *
 *         Currently only the MS8607 sensor is supported in this driver,
 *         but it is written to allow easy expansion to all of the sensors
 *         on the Weather Shield.
 */

#include "bsp.h"

#include <os.h>
#include <i2c.h>
#include <ms8607.h>
#include <stm32f7xx.h>

#include <stdlib.h>
#include <stdbool.h>

/*
 * Along with an I2C bus, the Weather Shield uses 3 GPIO pins:
 *     - Enable (active low): Turns on the mux.
 *     - Select A (active low): Mux select line.
 *     - Select B (active low): Mux select line.
 */
#define MUX_ENABLE_PORT   (GPIOD)
#define MUX_ENABLE_PIN    (GPIO_PIN_15)
#define MUX_SELECT_A_PORT (GPIOA)
#define MUX_SELECT_A_PIN  (GPIO_PIN_7)
#define MUX_SELECT_B_PORT (GPIOD)
#define MUX_SELECT_B_PIN  (GPIO_PIN_14)

static OS_MUTEX SensorMutex;

static void SelectSensor(Sensor_TypeDef sensor)
{
    switch (sensor)
    {
    case Sensor_MS8607:
        HAL_GPIO_WritePin(MUX_SELECT_A_PORT, MUX_SELECT_A_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MUX_SELECT_B_PORT, MUX_SELECT_B_PIN, GPIO_PIN_RESET);
        break;

    /* Bad input */
    default:
        break;
    }
}

static BSP_RESULT SensorPrologue(Sensor_TypeDef sensor)
{
    OS_ERR err;

    OSMutexPend((OS_MUTEX*) &SensorMutex,
                (OS_TICK)   0,
                (OS_OPT)    OS_OPT_PEND_BLOCKING,
                (CPU_TS*)   NULL,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    SelectSensor(sensor);

    return BSP_SUCCESS;
}

static BSP_RESULT SensorEpilogue(Sensor_TypeDef sensor)
{
    OS_ERR err;

    OSMutexPost((OS_MUTEX*) &SensorMutex,
                (OS_OPT)    OS_OPT_POST_NONE,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

/* Common initialization, only call this function from startup code (single task) */
BSP_RESULT BSP_Sensor_Init(void)
{
    OS_ERR err;
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable Weather Sheild GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Weather Shield GPIO configuration */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin   = MUX_ENABLE_PIN;
    HAL_GPIO_Init(MUX_ENABLE_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = MUX_SELECT_A_PIN;
    HAL_GPIO_Init(MUX_SELECT_A_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = MUX_SELECT_B_PIN;
    HAL_GPIO_Init(MUX_SELECT_B_PORT, &GPIO_InitStruct);

    /* Enable mux */
    HAL_GPIO_WritePin(MUX_ENABLE_PORT, MUX_ENABLE_PIN, GPIO_PIN_RESET);

    /* Create Sensor mutex, allowing multiple tasks to use BSP Sensor APIs safely */
    OSMutexCreate((OS_MUTEX*) &SensorMutex,
                  (CPU_CHAR*) "Sensor Mutex",
                  (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

/* Sensor specific initialization */
BSP_RESULT BSP_Sensor_Reset(Sensor_TypeDef sensor)
{
    OS_ERR err;
    BSP_RESULT result;

    if (SensorPrologue(sensor) != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    switch (sensor)
    {
    case Sensor_MS8607:
        ms8607_init();

        if (ms8607_is_connected() == false)
        {
            result = BSP_FAILURE;
            break;
        }

        if (ms8607_reset() != STATUS_OK)
        {
            result = BSP_FAILURE;
            break;
        }

        /* Small delay for MS8607 reset */
        OSTimeDlyHMSM((CPU_INT16U) 0,
                      (CPU_INT16U) 0,
                      (CPU_INT16U) 0,
                      (CPU_INT16U) 100,
                      (OS_OPT)     OS_OPT_TIME_HMSM_NON_STRICT | OS_OPT_TIME_DLY,
                      (OS_ERR*)    &err);

        if (err != OS_ERR_NONE)
        {
            result = BSP_FAILURE;
        }
        else
        {
            result = BSP_SUCCESS;
        }

        break;

    /* Bad input */
    default:
        result = BSP_FAILURE;
        break;
    }

    if (SensorEpilogue(sensor) != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    return result;
}

BSP_RESULT BSP_Sensor_Read(Sensor_TypeDef sensor, Sensor_Data* data)
{
    BSP_RESULT result;
    float temp, humid, press;

    if (data == NULL)
    {
        return BSP_FAILURE;
    }

    if (SensorPrologue(sensor) != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    switch (sensor)
    {
    case Sensor_MS8607:
        if (ms8607_read_temperature_pressure_humidity(&temp, &press, &humid) != STATUS_OK)
        {
            result = BSP_FAILURE;
            break;
        }
        else
        {
            result = BSP_SUCCESS;
        }

        data->temperature          = temp;
        data->temperature_is_valid = true;
        data->humidity             = humid;
        data->humidity_is_valid    = true;
        data->pressure             = press;
        data->pressure_is_valid    = true;

        break;

    /* Bad input */
    default:
        result = BSP_FAILURE;
        break;
    }

    if (SensorEpilogue(sensor) != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    return result;
}
