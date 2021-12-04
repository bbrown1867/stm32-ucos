/**
 * @file   main.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Application Entry Point.
 */

#include <app_task.h>
#include <logger_task.h>

#include <os.h>

#include <stddef.h>
#include <stdbool.h>

static void main_error_handler(bool expected)
{
    if (expected == false)
    {
        /*
         * Have not initializated RTOS or BSP yet, more complex error handling
         * is not possible at this point.
         */
        while (1);
    }
}

int main(void)
{
    OS_ERR err;

    /*
     * Recommended to only enable interrupts after the kernel has
     * started. Also recommended to do this within the BSP layer,
     * since other CPUs may have more complex interrupt disable
     * routines (uCOS-III The Real-Time Kernel: Page 70).
     */
    CPU_IntDis();

    OSInit(&err);
    main_error_handler(err == OS_ERR_NONE);

    /*
     * Initialize other kernel objects (memory pool, queue, mutex, etc).
     * Note that only task kernel objects are initialized here, BSP kernel
     * objects are initialized in the BSP for better code organization.
     */
    logger_init(&err);
    main_error_handler(err == OS_ERR_NONE);

    /*
     * Recommended to only enable a single task initially and then enable
     * others tasks from it (uCOS-III The Real-Time Kernel: Page 73).
     */
    app_create(&err);
    main_error_handler(err == OS_ERR_NONE);

    /* Should not return */
    OSStart(&err);
    main_error_handler(err == OS_ERR_NONE);

    return 1;
}
