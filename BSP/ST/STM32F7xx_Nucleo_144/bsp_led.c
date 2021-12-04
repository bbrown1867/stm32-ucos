/**
 * @file   bsp_led.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Thread-safe LED driver.
 *
 *         Note that stm32f7xx_nucleo_144.c in STM32CubeF7 has LED functionality,
 *         but that file is specifically not included to avoid multiple BSP layers
 *         in the project.
 *
 *         Mutual Exclusion Semaphores (mutex) are the preferred method of accessing
 *         shared resources in uCOS-III and it is recommended that library functions
 *         use mutex protection internally (uCOS-III The Real-Time Kernel: Pages 248,
 *         259). In this driver the GPIO registers are the shared resource and are
 *         protected by using a mutex.
 */

#include "bsp.h"

#include <os.h>
#include <stm32f7xx.h>

#include <stdlib.h>

static OS_MUTEX LedMutex;

static uint16_t LED_GetPin(LED_TypeDef led)
{
    if (led == LED_GREEN)
    {
        return GPIO_PIN_0;
    }
    else if (led == LED_RED)
    {
        return GPIO_PIN_7;
    }
    else
    {
        return GPIO_PIN_14;
    }
}

/* Only call this function from startup code (single task) */
BSP_RESULT BSP_LED_Init(void)
{
    OS_ERR err;
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable LED GPIO clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* LED GPIO pin configuration */
    GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Turn off all LEDs */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14, GPIO_PIN_RESET);

    /* Create LED mutex, allowing multiple tasks to use BSP LED APIs safely */
    OSMutexCreate((OS_MUTEX*) &LedMutex,
                  (CPU_CHAR*) "LED Mutex",
                  (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

BSP_RESULT BSP_LED_On(LED_TypeDef led)
{
    OS_ERR err;

    OSMutexPend((OS_MUTEX*) &LedMutex,
                (OS_TICK)   0,
                (OS_OPT)    OS_OPT_PEND_BLOCKING,
                (CPU_TS*)   NULL,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    HAL_GPIO_WritePin(GPIOB, LED_GetPin(led), GPIO_PIN_SET);

    OSMutexPost((OS_MUTEX*) &LedMutex,
                (OS_OPT)    OS_OPT_POST_NONE,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

BSP_RESULT BSP_LED_Off(LED_TypeDef led)
{
    OS_ERR err;

    OSMutexPend((OS_MUTEX*) &LedMutex,
                (OS_TICK)   0,
                (OS_OPT)    OS_OPT_PEND_BLOCKING,
                (CPU_TS*)   NULL,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    HAL_GPIO_WritePin(GPIOB, LED_GetPin(led), GPIO_PIN_RESET);

    OSMutexPost((OS_MUTEX*) &LedMutex,
                (OS_OPT)    OS_OPT_POST_NONE,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

BSP_RESULT BSP_LED_Toggle(LED_TypeDef led)
{
    OS_ERR err;

    OSMutexPend((OS_MUTEX*) &LedMutex,
                (OS_TICK)   0,
                (OS_OPT)    OS_OPT_PEND_BLOCKING,
                (CPU_TS*)   NULL,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    HAL_GPIO_TogglePin(GPIOB, LED_GetPin(led));

    OSMutexPost((OS_MUTEX*) &LedMutex,
                (OS_OPT)    OS_OPT_POST_NONE,
                (OS_ERR*)   &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}
