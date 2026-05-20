#ifndef __SENSOR_DRIVER_H
#define __SENSOR_DRIVER_H

#include "stm32f10x.h"
#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "stm32_clock.h"

extern volatile uint32_t record_tick_0;
extern volatile uint32_t distance_0;
extern volatile uint32_t record_tick_1;
extern volatile uint32_t distance_1;
extern volatile uint32_t record_tick_2;
extern volatile uint32_t distance_2;
extern volatile uint32_t record_tick_3;
extern volatile uint32_t distance_3;

void sensor_init();
void trig_0();

#endif // __SENSOR_DRIVER_H