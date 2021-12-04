/**
 * @file   logger_task.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  Logger Task.
 */

#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#include <os.h>

#include <stdint.h>

void logger_init     (OS_ERR* p_err);
void logger_create   (OS_ERR* p_err);
void logger_task     (void* p_arg);
void logger_log      (OS_TCB* p_tcb, OS_ERR* p_err, const char* p_msg);
void logger_log_int  (OS_TCB* p_tcb, OS_ERR* p_err, const char* p_msg, uint32_t value);
void logger_log_float(OS_TCB* p_tcb, OS_ERR* p_err, const char* p_msg, float value);

#endif /* LOGGER_TASK_H */
