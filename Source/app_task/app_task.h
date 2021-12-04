/**
 * @file   app_task.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  Application Task.
 */

#ifndef APP_TASK_H
#define APP_TASK_H

#include <os.h>

void app_create(OS_ERR* p_err);
void app_task  (void* p_arg);

#endif /* APP_TASK_H */
