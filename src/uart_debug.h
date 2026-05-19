#ifndef __UART_DEBUG_H
#define __UART_DEBUG_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdint.h>

void uart_debug_init(uint32_t baud);
void vofa_justfloat_send(float *data, uint8_t channel_count);

#endif
