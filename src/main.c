#include "main.h"
#include "chassis_control.h"

int main()
{
    RCC_Clock_Init();
    STM_Clock_Init(72);
    systick_init();
    STM_Timer3_Init();
    STM_CAN_Init();// 125kbps

    sensor_init();
    chassis_init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 启动电机
    start();

    while(1)
    {
        // 底盘状态机更新（包含传感器触发、状态判断、运动控制）
        chassis_update();

        // LED指示：正常前进时亮，遇到障碍物时灭
        if (chassis_get_state() == CHASSIS_STATE_FORWARD) {
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        } else {
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        }

        delay_ms(50);  // 50ms控制周期
    }
}
