/**
 * @file   main.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Application Entry Point.
 */

#include <app_task.h>
#include <os.h>
#include <stddef.h>

int main(void)
{
    OS_ERR err;

    /* 
     * Recommended to only enable interrupts after the kernel has
     * started. Also recommended to do this within the BSP layer,
     * since other CPUs may have more complex interrupt disable
     * routines (uCOS-III The Real-Time Kernel for STM32: Page 70).
     */
    CPU_IntDis();
    
    OSInit(&err);
    error_handler(err == OS_ERR_NONE);

    /* NOTE: Initialize other kernel objects here (queue, mutex, etc) */

    /* 
     * Recommended to only enable a single task initially and then enable
     * others tasks from it (uCOS-III The Real-Time Kernel for STM32: Page 73).
     */
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
                 (OS_ERR*)      &err);
    error_handler(err == OS_ERR_NONE);

    /* Should not return */
    OSStart(&err);
    error_handler(err == OS_ERR_NONE);

    return 1;
}
