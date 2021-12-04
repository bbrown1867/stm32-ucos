/**
 * @file   bsp.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Board Support Package.
 *
 *         The purpose of the BSP is to keep the uCOS application (everything in "Source") platform independent,
 *         allowing you to easily port the application to a different board, processor, or architecture.
 *         I don't know if Micrium has a definitive specification for how to organize the BSP, since I've seen
 *         a variety of structures in different projects. However here are some takeaways from the book:
 *
 *             - Have files named bsp.c and bsp.h.
 *
 *             - For folder structure use: <manufacturer>/<board_name>/<compiler>.
 *
 *             - bsp.c seems to have the `BSP_Init`, `BSP_CPU_ClkFreq` functions.
 *
 *             - Group related functionality into files, like bsp_led.c, bsp_uart.c. Have one header file,
 *               bsp.h, for all public prototypes. Application code includes this file.
 *
 *             - Place RTOS funtionality that the bsp_*.c drivers use into a file called OS/uCOS-III/bsp_os.c.
 *               This would have functions like semaphore pending/posting that drivers can use on blocking calls.
 *               This allows abstraction between the RTOS version and the BSP (e.g. uCOS-II, uCOS-III).
 *
 *             - Timestamping functionality goes into cpu_bsp.c: `CPU_TS_TmrInit`, `CPU_TS_TmrRd`. On Cortex-M
 *               this would use the CPU cycle count registers.
 *
 *         References:
 *             - uCOS-III The Real-Time Kernel (STM32 version, 2009): Pages 54, 70, 349, 753
 *             - Example project found on GitHub:
 *               https://github.com/ptracton/experimental/tree/master/C/STM32/RTOS/Micrium/Software/
 *
 *         TODO: Understanding the difference between `BSP_Tick_Init` (bsp.c) and `BSP_OS_TickInit` (bsp_os.c).
 */

#include "bsp.h"

#include <os.h>
#include <stm32f7xx.h>

#include <stdint.h>

static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable power control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Update the voltage scaling value */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 432;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    RCC_OscInitStruct.PLL.PLLR = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Activate the overdrive to reach the 216 MHz frequency */
    HAL_PWREx_EnableOverDrive();

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
}

BSP_RESULT BSP_Init(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();

    CPU_IntEn();
    SystemClock_Config();

    if (BSP_LED_Init() != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    if (BSP_Sensor_Init() != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    if (BSP_UART_Init() != BSP_SUCCESS)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

CPU_INT32U BSP_CPU_ClkFreq(void)
{
    SystemCoreClockUpdate();

    return (CPU_INT32U) SystemCoreClock;
}

void BSP_Tick_Init(void)
{
    OS_CPU_SysTickInitFreq(BSP_CPU_ClkFreq());
}

/*
 * STM32 HAL functions.
 *
 * NOTE:
 *
 *     The HAL drivers in STM32CubeF7 package do not support an RTOS. The stm32f7xx_hal.c
 *     driver will use the SysTick timer to give other drivers a way to delay/wait for a
 *     fixed amount of time using HAL_GetTick and HAL_Delay. This use of the SysTick timer
 *     will conflict with the RTOS use. To resolve this the file is not included in the
 *     build and required implementations are placed here, using the uCOS APIs as the
 *     source of time.
 *
 *     More important than this inconvenience is the fact that the bus drivers won't use RTOS
 *     primitives like semaphore post/pend for blocking operations. Unclear if or how this
 *     STM32CubeF7 package could be used in a more complex RTOS system. Perhaps people build
 *     their own BSP and STM32CubeF7 is just a reference or maybe Micrium and/or STM have
 *     an RTOS-aware STM32 BSP somewhere.
 *
 *     There is a USE_RTOS macro in stm32f7xx_hal_conf.h but it is unused and unsupported.
 */

uint32_t uwTickPrio;

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    /*
     * Called by HAL_RCC_ClockConfig to reconfigure the system tick after clock settings
     * have changed. This can be a "no-op" for us since we only call HAL_RCC_ClockConfig
     * once in SystemClock_Config and then configure the tick in BSP_Tick_Init after
     * using the up-to-date clock settings.
     */
    return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
    /* Ignore errors */
    OS_ERR err;

    return (uint32_t) OSTimeGet(&err);
}
