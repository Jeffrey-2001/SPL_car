#ifndef __PS2_DRIVER_H
#define __PS2_DRIVER_H

#include <stdint.h>
#include "stm32f10x.h"

// PS2手柄数据结构
typedef struct {
    uint8_t mode;        /* 手柄的工作模式 */

    uint8_t btn1;        /* B0:SLCT B1:JR  B0:JL B3:STRT B4:UP B5:R B6:DOWN  B7:L */

    uint8_t btn2;        /* B0:L2   B1:R2  B2:L1 B3:R1   B4:Y  B5:B B6:A     B7:X */

    uint8_t RJoy_LR;     /* 右摇杆左右  0x00 = 左    0xff = 右 */

    uint8_t RJoy_UD;     /* 右摇杆上下  0x00 = 上    0xff = 下 */

    uint8_t LJoy_LR;     /* 左摇杆左右  0x00 = 左    0xff = 右 */

    uint8_t LJoy_UD;     /* 左摇杆上下  0x00 = 上    0xff = 下 */

} JOYSTICK_TypeDef;

// 按键定义
#define PS2_BTN_SELECT   (1 << 0)  // btn1 bit0
#define PS2_BTN_JOYR     (1 << 1)  // btn1 bit1
#define PS2_BTN_JOYL     (1 << 2)  // btn1 bit2
#define PS2_BTN_START    (1 << 3)  // btn1 bit3
#define PS2_BTN_UP       (1 << 4)  // btn1 bit4
#define PS2_BTN_RIGHT    (1 << 5)  // btn1 bit5
#define PS2_BTN_DOWN     (1 << 6)  // btn1 bit6
#define PS2_BTN_LEFT     (1 << 7)  // btn1 bit7

#define PS2_BTN_L2       (1 << 0)  // btn2 bit0
#define PS2_BTN_R2       (1 << 1)  // btn2 bit1
#define PS2_BTN_L1       (1 << 2)  // btn2 bit2
#define PS2_BTN_R1       (1 << 3)  // btn2 bit3
#define PS2_BTN_Y        (1 << 4)  // btn2 bit4
#define PS2_BTN_B        (1 << 5)  // btn2 bit5
#define PS2_BTN_A        (1 << 6)  // btn2 bit6
#define PS2_BTN_X        (1 << 7)  // btn2 bit7

/**
 * @brief PS2手柄初始化
 * @note  配置PA4-PA7为SPI通信引脚
 */
void ps2_init(void);

/**
 * @brief PS2读取按键和摇杆值
 * @param JoystickStruct 手柄值结构体指针
 */
void ps2_scan(JOYSTICK_TypeDef* JoystickStruct);

/**
 * @brief 延时微秒
 * @param us 延时时间(微秒)
 */
void ps2_delay_us(uint32_t us);

#endif // __PS2_DRIVER_H
