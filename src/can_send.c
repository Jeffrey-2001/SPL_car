#include "can_send.h"

void CAN_SendDeviceInfor(void)
{
	CanTxMsg CANSendMsg = {0};
	
	CANSendMsg.ExtId = NTC_ID_Reply;
	CANSendMsg.IDE  = CAN_ID_EXT;
	CANSendMsg.RTR  = CAN_RTR_DATA;
	CANSendMsg.DLC  = 8;

    CANSendMsg.Data[0] = 0x00;
    CANSendMsg.Data[1] = 0x01;
    CANSendMsg.Data[2] = 0x02;
    CANSendMsg.Data[3] = 0x03;
    CANSendMsg.Data[4] = 0xFF;
    CANSendMsg.Data[5] = 0xFF;
    CANSendMsg.Data[6] = 0xFF;
    CANSendMsg.Data[7] = 0xFF;            
	CAN_Transmit(CAN1, &CANSendMsg);
}










