/**
 * @file   bsp_uart.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  RTOS-aware UART driver.
 *
 *         This driver uses a semaphore to allow the RTOS to context switch
 *         during a blocking (and potentially time consuming) UART transfer.
 *         This is effectively a "unilateral rendezvous", but the ISR is
 *         hidden from the task and owned by this driver (uCOS-III The
 *         Real-Time Kernel: Page 264).
 *
 *         Note that unlike bsp_led.c, this driver is not thread-safe. It
 *         should only be used by a single task.
 */

#include "bsp.h"

#include <os.h>
#include <stm32f7xx.h>

#include <stdlib.h>
#include <stdint.h>

/*
 * UART configuration for USART3, which is connected to the ST-Link
 * virtual COM port, allowing it to enumerate as a USB device on the
 * host computer.
 */
#define USARTx_IRQn                      USART3_IRQn
#define USARTx_IRQHandler                USART3_IRQHandler
#define USARTx                           USART3
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()
#define USARTx_TX_PIN                    GPIO_PIN_8
#define USARTx_TX_GPIO_PORT              GPIOD
#define USARTx_TX_GPIO_AF                GPIO_AF7_USART3
#define USARTx_RX_PIN                    GPIO_PIN_9
#define USARTx_RX_GPIO_PORT              GPIOD
#define USARTx_RX_GPIO_AF                GPIO_AF7_USART3

static UART_HandleTypeDef UartHandle;
static OS_SEM             UartSemaphore;

BSP_RESULT BSP_UART_Init(void)
{
    OS_ERR err;

    UartHandle.Instance          = USARTx;
    UartHandle.Init.BaudRate     = 115200;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        return BSP_FAILURE;
    }

    OSSemCreate(&UartSemaphore, "UART Semaphore", 0, &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

BSP_RESULT BSP_UART_Transmit(uint8_t* data, size_t size, OS_TICK timeout)
{
    OS_ERR err;

    /* TODO: Use DMA to generate fewer interrupts */
    if (HAL_UART_Transmit_IT(&UartHandle, data, size) != HAL_OK)
    {
        return BSP_FAILURE;
    }

    OSSemPend((OS_SEM*) &UartSemaphore,
              (OS_TICK) timeout,
              (OS_OPT)  OS_OPT_PEND_BLOCKING,
              (CPU_TS*) NULL,
              (OS_ERR*) &err);

    if (err != OS_ERR_NONE)
    {
        return BSP_FAILURE;
    }

    return BSP_SUCCESS;
}

/*
 * STM32 HAL functions.
 */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    OS_ERR err;

    OSSemPost((OS_SEM*) &UartSemaphore,
              (OS_OPT)  OS_OPT_POST_1,
              (OS_ERR*) &err);
}

void USARTx_IRQHandler(void)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    HAL_UART_IRQHandler(&UartHandle);

    OSIntExit();
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

    /* Enable UART GPIO clocks */
    USARTx_TX_GPIO_CLK_ENABLE();
    USARTx_RX_GPIO_CLK_ENABLE();

    /* Select SysClk as source of UART clock */
    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

    /* Enable UART clock */
    USARTx_CLK_ENABLE();

    /* Common UART GPIO pin configuration  */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

    /* TX UART GPIO pin configuration  */
    GPIO_InitStruct.Pin       = USARTx_TX_PIN;
    GPIO_InitStruct.Alternate = USARTx_TX_GPIO_AF;
    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* RX UART GPIO pin configuration  */
    GPIO_InitStruct.Pin       = USARTx_RX_PIN;
    GPIO_InitStruct.Alternate = USARTx_RX_GPIO_AF;
    HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

    /* Enable UART interrupts */
    HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    /* Reset UART clock */
    USARTx_FORCE_RESET();
    USARTx_RELEASE_RESET();

    /* Reset UART GPIO pin configurations */
    HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
    HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);

    /* Disable UART interrupts */
    HAL_NVIC_DisableIRQ(USARTx_IRQn);
}
