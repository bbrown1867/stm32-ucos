/**
 * @file   app_task.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Application Task.
 *
 *         The application task is responsible for creating other tasks
 *         in the system and maintaining an application heartbeat.
 */

#include "app_task.h"

#include <logger_task.h>
#include <sensor_task.h>

#include <os.h>
#include <bsp.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

static OS_TCB  AppTaskTCB;
static CPU_STK AppTaskStack[OS_CFG_APP_TASK_STK_SIZE];

static void app_error_handler(const char* msg, uint32_t actual, uint32_t expected)
{
    /* New errors are ignored, since there is no other course of action */
    OS_ERR err;

    if (actual != expected)
    {
        /* Attempt to log an error message */
        logger_log_int(&AppTaskTCB, &err, msg, actual);

        /* Attempt to turn on the red LED */
        (void) BSP_LED_On(LED_RED);

        /* Suspend the current task */
        OSTaskSuspend((OS_TCB*) NULL,
                      (OS_ERR*) &err);
    }
}

void app_create(OS_ERR* p_err)
{
    OSTaskCreate((OS_TCB*)      &AppTaskTCB,
                 (CPU_CHAR*)    "Application Task",
                 (OS_TASK_PTR)  app_task,
                 (void*)        NULL,
                 (OS_PRIO)      OS_CFG_APP_TASK_PRIO,
                 (CPU_STK*)     &AppTaskStack,
                 (CPU_STK_SIZE) OS_CFG_APP_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) OS_CFG_APP_TASK_STK_SIZE,
                 (OS_MSG_QTY)   0,
                 (OS_TICK)      0,
                 (void*)        0,
                 (OS_OPT)       OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR*)      p_err);
}

void app_task(void* p_arg)
{
    OS_ERR err;
    BSP_RESULT result;

    result = BSP_Init();
    app_error_handler("BSP_Init failed:", (uint32_t) result, (uint32_t) BSP_SUCCESS);

    CPU_Init();
    BSP_Tick_Init();

    /* Create logger task */
    logger_create(&err);
    app_error_handler("logger_create failed:", (uint32_t) err, (uint32_t) OS_ERR_NONE);

    /* Create sensor task */
    sensor_create(&err);
    app_error_handler("sensor_create failed:", (uint32_t) err, (uint32_t) OS_ERR_NONE);

    while (1)
    {
        result = BSP_LED_Toggle(LED_GREEN);
        app_error_handler("BSP_LED_Toggle failed:", (uint32_t) result, (uint32_t) BSP_SUCCESS);

        logger_log(&AppTaskTCB, &err, "App Task Heartbeat");
        app_error_handler("logger_log failed:", (uint32_t) err, (uint32_t) OS_ERR_NONE);

        OSTimeDly((OS_TICK) OS_CFG_APP_TASK_POLLING_INTERVAL,
                  (OS_OPT)  OS_OPT_TIME_DLY,
                  (OS_ERR*) &err);
        app_error_handler("OSTimeDly failed:", (uint32_t) err, (uint32_t) OS_ERR_NONE);
    }
}
