/**
 * @file   sensor_task.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Sensor Task.
 */

#include "sensor_task.h"

#include <logger_task.h>

#include <os.h>
#include <bsp.h>

#include <stdlib.h>
#include <stdbool.h>

static OS_TCB  SensorTaskTCB;
static CPU_STK SensorTaskStack[OS_CFG_SENSOR_TASK_STK_SIZE];

static void sensor_error_handler(const char* msg)
{
    /* New errors are ignored, since there is no other course of action */
    OS_ERR err;

    /* Attempt to log an error message */
    logger_log(&SensorTaskTCB, &err, msg);

    /* Attempt to turn on the red LED */
    (void) BSP_LED_On(LED_RED);

    /* Suspend the current task */
    OSTaskSuspend((OS_TCB*) NULL,
                  (OS_ERR*) &err);
}

void sensor_create(OS_ERR* p_err)
{
    OSTaskCreate((OS_TCB*)      &SensorTaskTCB,
                 (CPU_CHAR*)    "Sensor Task",
                 (OS_TASK_PTR)  sensor_task,
                 (void*)        NULL,
                 (OS_PRIO)      OS_CFG_SENSOR_TASK_PRIO,
                 (CPU_STK*)     &SensorTaskStack,
                 (CPU_STK_SIZE) OS_CFG_SENSOR_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) OS_CFG_SENSOR_TASK_STK_SIZE,
                 (OS_MSG_QTY)   0,
                 (OS_TICK)      0,
                 (void*)        0,
                 (OS_OPT)       OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR*)      p_err);
}

void sensor_task(void* p_arg)
{
    OS_ERR err;
    Sensor_Data data;
    uint32_t iterations;
    Sensor_TypeDef curr_sensor;

    /*
     * NOTE:
     *
     *     This task and bsp_sensor.c are not sensor specific, they are designed to support
     *     any `Sensor_TypeDef`. One nice feature of uCOS is that you can create multiple
     *     instances of the same task (e.g. `sensor_task`) that only differ in the data they
     *     operate (e.g. `p_arg`). A future improvement would be taking `curr_sensor` in as
     *     an argument once multiple sensors are supported in bsp_sensor.c.
     */
    curr_sensor = Sensor_MS8607;

    /* Initialize locals */
    iterations = 0;

    data.temperature_is_valid = false;
    data.humidity_is_valid    = false;
    data.pressure_is_valid    = false;

    if (BSP_Sensor_Reset(curr_sensor) != BSP_SUCCESS)
    {
        sensor_error_handler("Failed to reset sensor");
    }

    while (1)
    {
        /* Read sensor */
        if (BSP_Sensor_Read(curr_sensor, &data) != BSP_SUCCESS)
        {
            sensor_error_handler("Failed to read sensor");
        }

        /* Log sensor data */
        if (data.temperature_is_valid == true)
        {
            logger_log_float(&SensorTaskTCB, &err, "Temperature:", data.temperature);

            if (err != OS_ERR_NONE)
            {
                sensor_error_handler("Failed to log temperature");
            }
        }

        if (data.humidity_is_valid == true)
        {
            logger_log_float(&SensorTaskTCB, &err, "Humidity:", data.humidity);

            if (err != OS_ERR_NONE)
            {
                sensor_error_handler("Failed to log humidity");
            }
        }

        if (data.pressure_is_valid == true)
        {
            logger_log_float(&SensorTaskTCB, &err, "Pressure:", data.pressure);

            if (err != OS_ERR_NONE)
            {
                sensor_error_handler("Failed to log pressure");
            }
        }

        /* Track number of times the sensor has been read */
        iterations++;
        logger_log_int(&SensorTaskTCB, &err, "Number of Sensor Readings =", iterations);

        /* Delay for polling interval */
        OSTimeDly((OS_TICK) OS_CFG_SENSOR_TASK_POLLING_INTERVAL,
                  (OS_OPT)  OS_OPT_TIME_DLY,
                  (OS_ERR*) &err);

        if (err != OS_ERR_NONE)
        {
            sensor_error_handler("Failed to poll sensor");
        }
    }
}
