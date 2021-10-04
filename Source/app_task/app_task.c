/**
 * @file   app_task.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Application Task.
 *
 *         The application task is responsible for creating other tasks in the system
 *         and managing high-level system events such as errors.
 */

#include "app_task.h"

#include <bsp.h>
#include <os.h>
#include <stdbool.h>
#include <stdint.h>

OS_TCB  AppTaskTCB;
CPU_STK AppTaskStack[OS_CFG_APP_TASK_STK_SIZE];

void app_task(void* arg)
{
    OS_ERR err;

    BSP_Init();

    CPU_Init();
    BSP_Tick_Init();

    while (1)
    {
        BSP_LED_Toggle(LED_GREEN);

        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
        error_handler(err == OS_ERR_NONE);
    }
}

void error_handler(bool expected)
{
    if (expected == false)
    {
        BSP_LED_On(LED_RED);
        while (1);
    }
}
