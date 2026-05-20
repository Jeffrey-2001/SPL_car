#ifndef _STM32_CAN_H_
#define _STM32_CAN_H_

#include "stm32f10x.h"

extern volatile CanRxMsg CANRecMsg;
extern volatile uint8_t  CANRecFlag;

#define NTC_ID_Request1 	0x0AB03DF0
#define NTC_ID_Reply 		0x0AB5F03D

void STM_CAN_Init(void);


#endif










