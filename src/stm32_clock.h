#ifndef _STM32_CLOCK_H_
#define _STM32_CLOCK_H_

#include "stm32f10x.h"

uint32_t RCC_Clock_Init(void);  // 返回系统时钟频率(MHz)

// 系统时钟(单位：M)
void STM_Clock_Init(uint16_t systick);

void STM_Delayms(uint16_t nms);
void delay_ms(uint16_t nms);
void systick_init(uint32_t sysclk_mhz);
uint32_t get_tick_ms();

#endif

