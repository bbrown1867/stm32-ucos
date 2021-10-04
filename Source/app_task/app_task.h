/**
 * @file   app_task.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  Application Task.
 */

#ifndef APP_TASK_H
#define APP_TASK_H

#include <os.h>
#include <stdbool.h>

extern OS_TCB  AppTaskTCB;
extern CPU_STK AppTaskStack[OS_CFG_APP_TASK_STK_SIZE];

void app_task(void* arg);
void error_handler(bool expected);

#endif /* APP_TASK_H */
