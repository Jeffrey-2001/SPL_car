#ifndef _STM32_WATCHDOG_H_
#define _STM32_WATCHDOG_H_

#include "stm32f10x.h"

#include "basic_data.h"

#include "stm32_clock.h"

#include "stm32_timer3.h"


void STM_Watchdog_Init(void);
void STM_Watchdog_Feed(void);

#endif




