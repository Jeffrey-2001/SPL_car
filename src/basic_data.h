#ifndef _BASIC_DATA_H_
#define _BASIC_DATA_H_

#include "stm32f10x.h"

typedef struct
{
	uint16_t CANTime;
}NTCInfor_t;
extern NTCInfor_t NTCInfor;

extern volatile uint32_t Sys_1ms;     // 系统毫秒计数器
extern volatile uint8_t flag_1ms;     // 1ms 任务标志
extern volatile uint8_t flag_10ms;    // 10ms 任务标志 (PS2扫描)
extern volatile uint8_t flag_50ms;    // 50ms 任务标志 (VOFA发送)
extern volatile uint8_t flag_100ms;   // 100ms 任务标志 (心跳/LED)

#endif













