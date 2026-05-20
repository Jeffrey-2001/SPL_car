#ifndef __MOTOR_DRIVER_H
#define __MOTOR_DRIVER_H

#include <stdint.h>
#include "stm32f10x.h"
#include "stm32_can.h"
#include "basic_data.h"

#define MASTER_BASE_ID 0x600
#define STOP_EMERGENCY_CMD {0x2b, 0x40, 0x40, 0x00, 0x01, 0x00, 0x00, 0x00}
#define STOP_FREE_CMD {0x2b, 0x40, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00}
#define STOP_NORMAL_CMD {0x2b, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00}
#define RUN_CMD {0x2b, 0x01, 0x20, 0x00, 0xf4, 0x01, 0x00, 0x00}

void send_data(uint32_t StdId, uint8_t* datas);
void stop_emergency(uint32_t device_id);
void stop_free(uint32_t device_id);
void stop_normal(uint32_t device_id);

void set_speed(uint32_t device_id, uint16_t speed);
void set_pwm(uint32_t device_id, uint16_t pwm);

void turn_left(uint32_t left_id, uint32_t right_id, uint16_t speed);
void turn_right(uint32_t left_id, uint32_t right_id, uint16_t speed);
void stop_turning(uint32_t left_id, uint32_t right_id);
void forward(uint32_t left_id, uint32_t right_id, uint16_t speed);
void backward(uint32_t left_id, uint32_t right_id, uint16_t speed);

#endif
