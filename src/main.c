#include "main.h"
#include "ax_ps2.h"
#include "ax_sys.h"    //系统设置
#include "ax_delay.h"  //软件延时
#include "ax_led.h"    //LED灯控制
#include "ax_uart1.h"  //调试串口
#include "stm32_can.h" //CAN通信
#include "motor_driver.h"
#include "stm32_clock.h"
#include "ax_ps2.h" //PS2手柄


JOYSTICK_TypeDef my_joystick;  //手柄键值结构体

int main()
{   
    RCC_Clock_Init();
    STM_Clock_Init(72);
    STM_CAN_Init();

     //延时函数初始化
    AX_DELAY_Init();   
    sensor_init();
     //LED初始化
    AX_LED_Init();
    // //调试串口初始化
    AX_UART1_Init(115200);
    Timer_Init();


    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    AX_PS2_Init(); // PS2手柄初始化
    
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

        // trig_0();        
        // delay_ms(100) ;
        // if (distance_0 < 500) /* 5米内才执行避障算法，过远可能是测距错误，不要盲目避障 */
        // {
        //     if (!stop){
        //         stop_emergency(0x01); // 紧急停止
        //         stop = 1  ; // 设置停止标志，避免重复调用紧急停止函数
        //     }
        //     GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        // }else{
        //     if(stop){
        //         stop = 0; // 清除停止标志
        //         set_speed(0x01, 500); // 恢复速度，继续前进
        //     }
        //     GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        // }
		
        // delay_ms(100) ; 
    }

}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        AX_PS2_ScanKey(&my_joystick);
        if (my_joystick.btn1 & 0x80)
        {
            turn_left(0x01, 0x02, 200);
        }
        else if (my_joystick.btn1 & 0x40)
        {
            backward(0x01, 0x02, 200);
        }
        else if (my_joystick.btn1 & 0x20)
        {
            turn_right(0x01, 0x02, 200);
        }
        else if (my_joystick.btn1 & 0x10)
        {
            forward(0x01, 0x02, 200);
        }
        else
        {
            set_speed(0x01, 0);
            STM_Delayms(30);
            set_speed(0x02, 0);
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}



