#include "motor_driver.h"

void send_data(uint32_t StdId, uint8_t* datas){
	CanTxMsg CANSendMsg = {0};
	
    CANSendMsg.StdId = StdId;
	CANSendMsg.ExtId = 0x00;
	CANSendMsg.IDE  = CAN_ID_STD;
	CANSendMsg.RTR  = CAN_RTR_DATA;
	CANSendMsg.DLC  = 8;

    for(int i=0; i<8; i++){
        CANSendMsg.Data[i] = datas[i];
    }

	CAN_Transmit(CAN1, &CANSendMsg);
}

void start_device(uint32_t device_id){
    uint32_t id = 0x000 + device_id;
    uint8_t cmd[8] = {0x01, 0x01, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00};
    send_data(id, cmd);
}

void start(){
    start_device(001);
    start_device(002);
}

void stop_emergency(uint32_t device_id){
    uint32_t id = MASTER_BASE_ID + device_id;
    uint8_t cmd[8] = STOP_EMERGENCY_CMD;
    send_data(id, cmd);
}

void stop_free(uint32_t device_id){
    uint32_t id = MASTER_BASE_ID + device_id;
    uint8_t cmd[8] = STOP_FREE_CMD;
    send_data(id, cmd);
}

void stop_normal(uint32_t device_id){
    uint32_t id = MASTER_BASE_ID + device_id;
    uint8_t cmd[8] = STOP_NORMAL_CMD;
    send_data(id, cmd);
}

void run(uint32_t device_id){
    uint32_t id = MASTER_BASE_ID + device_id;
    uint8_t cmd[8] = RUN_CMD;
    send_data(id, cmd);
}

void set_speed(uint32_t device_id, uint16_t speed){
    uint32_t id = MASTER_BASE_ID + device_id;
    uint8_t cmd[8] = {0x2b, 0x43, 0x40 ,0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[4] = (uint8_t)speed;
    cmd[5] = (uint8_t)(speed>>8);
    send_data(id, cmd);
}

void set_pwm(uint32_t device_id, uint16_t pwm){
    uint32_t id = MASTER_BASE_ID + device_id;
    uint8_t cmd[8] = {0x2b, 0x42, 0x40 ,0x00, 0x00, 0x00, 0x00, 0x00};
    cmd[4] = (uint8_t)pwm;
    cmd[5] = (uint8_t)(pwm>>8);
    send_data(id, cmd);
}

void turn_left(uint32_t left_id, uint32_t right_id, uint16_t speed){
    set_speed(left_id, speed);
    set_speed(right_id, -speed);
}

void turn_right(uint32_t left_id, uint32_t right_id, uint16_t speed){
    set_speed(left_id, -speed);
    set_speed(right_id, speed);
}

void stop_turning(uint32_t left_id, uint32_t right_id){
    stop_normal(left_id);
    stop_normal(right_id);
}