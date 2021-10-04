/**
 * @file   bsp.h
 * @author Ben Brown <ben@beninter.net>
 * @brief  Board Support Package.
 */

#ifndef NUCLEO_144_BSP_H
#define NUCLEO_144_BSP_H

#include <os.h>

typedef enum
{
    LED_GREEN,
    LED_BLUE,
    LED_RED,
} LED_TypeDef;

void       BSP_Init       (void);
CPU_INT32U BSP_CPU_ClkFreq(void);
void       BSP_Tick_Init  (void);

void       BSP_LED_Init   (LED_TypeDef led);
void       BSP_LED_On     (LED_TypeDef led);
void       BSP_LED_Off    (LED_TypeDef led);
void       BSP_LED_Toggle (LED_TypeDef led);

#endif /* NUCLEO_144_BSP_H */
