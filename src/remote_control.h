#ifndef __REMOTE_CONTROL_H
#define __REMOTE_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "chassis_control.h"
#include "ps2_driver.h"

// 遥控速度范围
#define REMOTE_SPEED_MIN 0
#define REMOTE_SPEED_MAX 1000
#define REMOTE_SPEED_DEFAULT 500

/**
 * @brief 遥控初始化
 * @note  初始化PS2手柄和遥控参数
 */
void remote_init(void);

/**
 * @brief 遥控更新（主循环调用）
 * @note  扫描手柄数据，更新状态和速度
 */
void remote_update(void);

/**
 * @brief 获取手柄期望的底盘状态
 * @return 底盘状态枚举值
 */
chassis_state_t remote_get_state(void);

/**
 * @brief 获取手柄期望的速度值
 * @return 速度值（正负分别表示前进/后退，0表示停止）
 */
int16_t remote_get_speed(void);

/**
 * @brief 获取手柄期望的转向速度值
 * @return 转向速度值（0表示不转向）
 */
int16_t remote_get_turn_speed(void);

/**
 * @brief 获取是否按下紧急停止按钮
 * @return true: 紧急停止, false: 正常
 */
bool remote_is_emergency_stop(void);

/**
 * @brief 获取当前遥控速度档位
 * @return 速度档位(1-5)
 */
uint8_t remote_get_speed_level(void);

/**
 * @brief 检测是否按下SELECT按键
 * @return true: 按下, false: 未按下
 */
bool remote_is_select_pressed(void);

/**
 * @brief 获取PS2手柄原始数据
 * @return PS2数据结构指针
 */
const JOYSTICK_TypeDef* remote_get_ps2_data(void);

#endif // __REMOTE_CONTROL_H
