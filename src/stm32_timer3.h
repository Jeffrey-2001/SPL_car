#ifndef _STM32_TIMER3_H_
#define _STM32_TIMER3_H_

#include "stm32f10x.h"

#include "basic_data.h"

extern volatile uint64_t Sys_1ms;
void STM_Timer3_Init(void);

#endif

