#include "main.h"
#include "chassis_control.h"
#include "remote_control.h"
#include "uart_debug.h"
#include <stdio.h>

int main()
{
    uint32_t sysclk = RCC_Clock_Init();
    STM_Clock_Init(sysclk);
    systick_init(sysclk);
    STM_Timer3_Init();
    STM_CAN_Init();// 125kbps

    sensor_init();
    chassis_init();
    remote_init();

    // UART调试串口 (VOFA+)
    uart_debug_init(115200);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 启动LED快闪3次，确认系统已运行
    for (int i = 0; i < 3; i++) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);  // LED亮
        delay_ms(100);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);    // LED灭
        delay_ms(100);
    }

    // 启动电机
    start();
    turn_left(0x01, 0x02, 200);  // 初始转向，测试电机通信
    //ps2_delay_us(300000000);  // 等待半秒，观察电机响应
    //stop_emergency(0x01);
    //  ps2_delay_us(300000000); 
    //stop_emergency(0x00);

    //手柄部分代码
    while(1)
    {
        // // PS2手柄扫描和遥控更新
        // remote_update();

        // // 发送PS2手柄数据到VOFA+ (使用JustFloat协议)
        // {
        //     const JOYSTICK_TypeDef* ps2 = remote_get_ps2_data();
        //     float vofa_data[7];
        //     vofa_data[0] = (float)ps2->mode;
        //     vofa_data[1] = (float)ps2->btn1;
        //     vofa_data[2] = (float)ps2->btn2;
        //     vofa_data[3] = (float)ps2->RJoy_LR;
        //     vofa_data[4] = (float)ps2->RJoy_UD;
        //     vofa_data[5] = (float)ps2->LJoy_LR;
        //     vofa_data[6] = (float)ps2->LJoy_UD;
        //     vofa_justfloat_send(vofa_data, 7);
        // }

        // // 检测SELECT键切换模式
        // static bool select_pressed = false;
        // bool select_current = remote_is_select_pressed();

        // if (select_current && !select_pressed) {
        //     // 检测到SELECT键按下，切换模式
        //     chassis_toggle_mode();
        // }
        // select_pressed = select_current;

        // // 根据遥控模块的状态更新底盘
        // if (chassis_get_mode() == CHASSIS_MODE_REMOTE) {
        //     // 遥控模式：根据手柄输入设置底盘状态和速度
        //     chassis_state_t remote_state = remote_get_state();
        //     chassis_set_state(remote_state);

        //     // 紧急停止检测
        //     if (remote_is_emergency_stop()) {
        //         chassis_emergency_stop();
        //     }

        //     // LED指示：遥控模式时慢闪
        //     static uint32_t led_counter = 0;
        //     led_counter++;
        //     if (led_counter % 10 < 5) {
        //         GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        //     } else {
        //         GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        //     }
        // } else {
        //     // 自动避障模式
        //     chassis_update();

        //     // LED指示：正常前进时亮，遇到障碍物时灭
        //     if (chassis_get_state() == CHASSIS_STATE_FORWARD) {
        //         GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        //     } else {
        //         GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        //     }
        // }

        delay_ms(50);  // 50ms控制周期
    }
}
