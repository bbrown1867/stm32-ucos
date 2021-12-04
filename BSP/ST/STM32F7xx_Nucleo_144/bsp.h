/**
 * @file   bsp.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  Board Support Package.
 */

#ifndef NUCLEO_144_BSP_H
#define NUCLEO_144_BSP_H

#include <os.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define BSP_SUCCESS (0U)
#define BSP_FAILURE (1U)

typedef uint8_t BSP_RESULT;

typedef enum
{
    Sensor_MS8607,
} Sensor_TypeDef;

typedef struct
{
    float temperature;
    bool  temperature_is_valid;
    float humidity;
    bool  humidity_is_valid;
    float pressure;
    bool  pressure_is_valid;
} Sensor_Data;

typedef enum
{
    LED_GREEN,
    LED_BLUE,
    LED_RED,
} LED_TypeDef;

/* bsp.c */
BSP_RESULT BSP_Init       (void);
CPU_INT32U BSP_CPU_ClkFreq(void);
void       BSP_Tick_Init  (void);

/* bsp_led.c */
BSP_RESULT BSP_LED_Init  (void);
BSP_RESULT BSP_LED_On    (LED_TypeDef led);
BSP_RESULT BSP_LED_Off   (LED_TypeDef led);
BSP_RESULT BSP_LED_Toggle(LED_TypeDef led);

/* bsp_sensor.c */
BSP_RESULT BSP_Sensor_Init (void);
BSP_RESULT BSP_Sensor_Reset(Sensor_TypeDef sensor);
BSP_RESULT BSP_Sensor_Read (Sensor_TypeDef sensor, Sensor_Data* data);

/* bsp_uart.c */
BSP_RESULT BSP_UART_Init    (void);
BSP_RESULT BSP_UART_Transmit(uint8_t* data, size_t size, OS_TICK timeout);

#endif /* NUCLEO_144_BSP_H */
