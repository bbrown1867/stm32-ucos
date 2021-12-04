/**
 * @file   sensor_task.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  Sensor Task.
 */

#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include <os.h>

void sensor_create(OS_ERR* p_err);
void sensor_task  (void* p_arg);

#endif /* APP_TASK_H */
