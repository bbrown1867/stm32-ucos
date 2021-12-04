/**
 * @file   logger_task.c
 * @author Ben Brown <ben@beninter.net>
 * @brief  Logger Task.
 *
 *         The logger task is the exclusive owner of the logging hardware,
 *         located in bsp_uart.c. Other tasks in the system can log messages
 *         by calling the `logger_log` APIs. These APIs will send a message to
 *         the logger task containing the log message and the logger task will
 *         transmit it using bsp_uart.c.
 */

#include "logger_task.h"

#include <os.h>
#include <bsp.h>

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#define TIMEOUT_TICKS   (1000U)
#define NUM_LOG_BUFFERS (16U)
#define LOG_BUF_SIZE    (128U)
#define TMP_BUF_SIZE    (64U)

static OS_TCB  LoggerTaskTCB;
static CPU_STK LoggerTaskStack[OS_CFG_LOGGER_TASK_STK_SIZE];

static OS_MEM LogMem;
static char   LogBuf[NUM_LOG_BUFFERS][LOG_BUF_SIZE];

void logger_create(OS_ERR* p_err)
{
    OSTaskCreate((OS_TCB*)      &LoggerTaskTCB,
                 (CPU_CHAR*)    "Logger Task",
                 (OS_TASK_PTR)  logger_task,
                 (void*)        NULL,
                 (OS_PRIO)      OS_CFG_LOGGER_TASK_PRIO,
                 (CPU_STK*)     &LoggerTaskStack,
                 (CPU_STK_SIZE) OS_CFG_LOGGER_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE) OS_CFG_LOGGER_TASK_STK_SIZE,
                 (OS_MSG_QTY)   OS_CFG_LOGGER_TASK_QUEUE_SIZE,
                 (OS_TICK)      0,
                 (void*)        0,
                 (OS_OPT)       OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR*)      p_err);
}

void logger_init(OS_ERR* p_err)
{
    OSMemCreate((OS_MEM*)     &LogMem,
                (CPU_CHAR*)   "Log Buffers",
                (void*)       LogBuf,
                (OS_MEM_QTY)  NUM_LOG_BUFFERS,
                (OS_MEM_SIZE) LOG_BUF_SIZE * sizeof(char),
                (OS_ERR*)     p_err);
}

void logger_task(void* p_arg)
{
    while (1)
    {
        OS_ERR err;
        uint8_t* p_msg;
        BSP_RESULT result;
        OS_MSG_SIZE msg_size;

        /* Wait for other tasks to send messages to log */
        p_msg = (uint8_t*) OSTaskQPend((OS_TICK)      0,
                                       (OS_OPT)       OS_OPT_PEND_BLOCKING,
                                       (OS_MSG_SIZE*) &msg_size,
                                       (CPU_TS*)      NULL,
                                       (OS_ERR*)      &err);

        if (err == OS_ERR_NONE)
        {
            /* Log the message using the RTOS-aware UART driver */
            result = BSP_UART_Transmit(p_msg, msg_size, TIMEOUT_TICKS);

            if (result != BSP_SUCCESS)
            {
                /*
                 * Signal that an error occurred, but don't suspend the current
                 * task because:
                 *  - We need to return the buffer to the memory pool.
                 *  - The UART error may have been spurious and non-fatal.
                 */
                (void) BSP_LED_On(LED_RED);
            }

            /* Return the log message buffer to the memory pool */
            OSMemPut((OS_MEM*) &LogMem,
                     (void*)   p_msg,
                     (OS_ERR*) &err);

            if (err != OS_ERR_NONE)
            {
                /*
                 * Signal that an error occurred and suspend the current
                 * task because we now have a dangling pointer from the
                 * memory pool (memory leak).
                 */
                (void) BSP_LED_On(LED_RED);
                OSTaskSuspend((OS_TCB*) NULL,
                              (OS_ERR*) &err);
            }
        }
    }
}

void logger_log(OS_TCB* p_tcb, OS_ERR* p_err, const char* p_msg)
{
    void* p_buf;
    int n_chars;
    uint32_t curr_time;
    OS_ERR ignored_error;

    curr_time = (uint32_t) OSTimeGet(p_err);

    if (*p_err == OS_ERR_NONE)
    {
        p_buf = OSMemGet((OS_MEM*) &LogMem,
                         (OS_ERR*) p_err);

        if (*p_err == OS_ERR_NONE)
        {
            n_chars = snprintf(p_buf, LOG_BUF_SIZE, "[%lu][%s] %s\n", curr_time, p_tcb->NamePtr, p_msg);

            /*
             * If we run out of buffer space, we will not raise an error and
             * just log the trimmed message. More robust error handling
             * would check that chars_written is also less than
             * (LOG_BUF_SIZE - 1) to be certain it will fit in p_buf.
             */
            if (n_chars > 0)
            {
                OSTaskQPost((OS_TCB*)     &LoggerTaskTCB,
                            (void*)       p_buf,
                            (OS_MSG_SIZE) n_chars,
                            (OS_OPT)      OS_OPT_POST_FIFO,
                            (OS_ERR*)     p_err);

                if (*p_err == OS_ERR_NONE)
                {
                    /* Success! */
                    return;
                }
            }
            else
            {
                /* Indicate sprintf error to caller */
                *p_err = OS_ERR_OPT_INVALID;
            }

            /* If a failure happens after we called OSMemGet, put the buffer back */
            OSMemPut((OS_MEM*) &LogMem,
                     (void*)   p_buf,
                     (OS_ERR*) &ignored_error);
        }
    }
}

/* Warning: Places a fairly large buffer (TMP_BUF_SIZE) on the calling task's stack */
void logger_log_int(OS_TCB* p_tcb, OS_ERR* p_err, const char* p_msg, uint32_t value)
{
    int n_chars;
    char temp_buf[TMP_BUF_SIZE];

    n_chars = snprintf(temp_buf, TMP_BUF_SIZE, "%s %lu", p_msg, value);

    if (n_chars > 0)
    {
        logger_log(p_tcb, p_err, temp_buf);
    }
    else
    {
        /* Indicate sprintf error to caller */
        *p_err = OS_ERR_OPT_INVALID;
    }
}

/* Warning: Places a fairly large buffer (TMP_BUF_SIZE) on the calling task's stack */
void logger_log_float(OS_TCB* p_tcb, OS_ERR* p_err, const char* p_msg, float value)
{
    int n_chars;
    char temp_buf[TMP_BUF_SIZE];

    n_chars = snprintf(temp_buf, TMP_BUF_SIZE, "%s %f", p_msg, value);

    if (n_chars > 0)
    {
        logger_log(p_tcb, p_err, temp_buf);
    }
    else
    {
        /* Indicate sprintf error to caller */
        *p_err = OS_ERR_OPT_INVALID;
    }
}
