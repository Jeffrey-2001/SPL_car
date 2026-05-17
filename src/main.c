#include "main.h"

uint32_t stop = 0; // 定义停止标志

int main()
{   
    RCC_Clock_Init();
    STM_Clock_Init(72);
    systick_init();
	STM_Timer3_Init();
    STM_CAN_Init();// 125kbps

    sensor_init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
	
    while(1)
	{ 	
		// // 与主控通信
		// if(NTCInfor.CANTime >= 1000)
		// {

		// }
        // turn_left(0x01, 0x02, 400);
        // delay_ms(1000);
        // stop_turning(0x01, 0x02);
        // delay_ms(2000);
        // turn_right(0x01, 0x02, 400);
        // delay_ms(1000);
        // stop_turning(0x01, 0x02);
        // delay_ms(2000);

        trig_0();        
        delay_ms(100) ;
        if (distance_0 < 500) /* 5米内才执行避障算法，过远可能是测距错误，不要盲目避障 */
        {
            if (!stop){
                stop_emergency(0x01); // 紧急停止
                stop = 1  ; // 设置停止标志，避免重复调用紧急停止函数
            }
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        }else{
            if(stop){
                stop = 0; // 清除停止标志
                set_speed(0x01, 500); // 恢复速度，继续前进
            }
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        }
		
        delay_ms(100) ; 
    }
}

