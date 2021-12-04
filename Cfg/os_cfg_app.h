/*
*********************************************************************************************************
*                                              uC/OS-III
*                                        The Real-Time Kernel
*
*                    Copyright 2009-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                               OS CONFIGURATION (APPLICATION SPECIFICS)
*
* Filename : os_cfg_app.h
* Version  : V3.08.01
*********************************************************************************************************
*/

#ifndef OS_CFG_APP_H
#define OS_CFG_APP_H

/*
**************************************************************************************************************************
*                                                      CONSTANTS
**************************************************************************************************************************
*/
                                                                /* ------------------ MISCELLANEOUS ------------------- */
                                                                /* Stack size of ISR stack (number of CPU_STK elements) */
#define  OS_CFG_ISR_STK_SIZE                             128u
                                                                /* Maximum number of messages                           */
#define  OS_CFG_MSG_POOL_SIZE                             32u
                                                                /* Stack limit position in percentage to empty          */
#define  OS_CFG_TASK_STK_LIMIT_PCT_EMPTY                  10u


                                                                /* -------------------- IDLE TASK --------------------- */
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_IDLE_TASK_STK_SIZE                        64u


                                                                /* ------------------ STATISTIC TASK ------------------ */
                                                                /* Priority                                             */
#define  OS_CFG_STAT_TASK_PRIO  ((OS_PRIO)(OS_CFG_PRIO_MAX-2u))
                                                                /* Rate of execution (1 to 10 Hz)                       */
#define  OS_CFG_STAT_TASK_RATE_HZ                         10u
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_STAT_TASK_STK_SIZE                       100u


                                                                /* ---------------------- TICKS ----------------------- */
                                                                /* Tick rate in Hertz (10 to 1000 Hz)                   */
#define  OS_CFG_TICK_RATE_HZ                            1000u


                                                                /* --------------------- TIMERS ----------------------- */
                                                                /* Priority of 'Timer Task'                             */
#define  OS_CFG_TMR_TASK_PRIO   ((OS_PRIO)(OS_CFG_PRIO_MAX-3u))
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_TMR_TASK_STK_SIZE                        128u

                                                                /* DEPRECATED - Rate for timers (10 Hz Typ.)            */
                                                                /* The timer task now calculates its timeouts based     */
                                                                /* on the timers in the list. It no longer runs at a    */
                                                                /* static frequency.                                    */
                                                                /* This define is included for compatibility reasons.   */
                                                                /* It will determine the period of a timer tick.        */
                                                                /* We recommend setting it to OS_CFG_TICK_RATE_HZ       */
                                                                /* for new projects.                                    */
#define  OS_CFG_TMR_TASK_RATE_HZ                          10u

/*
**************************************************************************************************************************
*                                                      APPLICATION
**************************************************************************************************************************
*/

/*
 * Notes on priorities:
 *     - 0 is the highest priority level.
 *     - (OS_CFG_PRIO_MAX - 1) is the lowest priority level.
 *     - Multiple tasks can use the same priority level (round-robin).
 *
 * Internal tasks:
 *     - Idle task is lowest priority: (OS_CFG_PRIO_MAX - 1), no other task may use this priority level.
 *     - Stats task is second lowest priority: (OS_CFG_PRIO_MAX - 2), but this could be changed.
 *     - Timer task is third lowest priority: (OS_CFG_PRIO_MAX - 3), but this could be changed.
 *     - Tick task is usually set to a high priority (uCOS-III The Real-Time Kernel: Page 122),
 *       however the OS_CFG_TICK_TASK_PRIO macro does not appear anywhere in the uCOS source code
 *       so maybe this internal task has been deprecated.
 */

                                                                /* ----------------- APPLICATION TASK ----------------- */
                                                                /* Priority of 'Application Task'                       */
#define  OS_CFG_APP_TASK_PRIO                    ((OS_PRIO) 0)
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_APP_TASK_STK_SIZE                        512u
                                                                /* Polling interval (in number of OS_TICK elements)     */
#define  OS_CFG_APP_TASK_POLLING_INTERVAL               1000u

                                                                /* -------------------- LOGGER TASK ------------------- */
                                                                /* Priority of 'Logger Task'                            */
#define  OS_CFG_LOGGER_TASK_PRIO                 ((OS_PRIO) 2)
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_LOGGER_TASK_STK_SIZE                     512u
                                                                /* Task message queue size for 'Logger Task'            */
#define  OS_CFG_LOGGER_TASK_QUEUE_SIZE                    20u

                                                                /* -------------------- SENSOR TASK ------------------- */
                                                                /* Priority of 'Sensor Task'                            */
#define  OS_CFG_SENSOR_TASK_PRIO                 ((OS_PRIO) 1)
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_SENSOR_TASK_STK_SIZE                     512u
                                                                /* Polling interval (in number of OS_TICK elements)     */
#define  OS_CFG_SENSOR_TASK_POLLING_INTERVAL            1000u

#endif
