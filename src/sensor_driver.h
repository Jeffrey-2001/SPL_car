#ifndef __SENSOR_DRIVER_H
#define __SENSOR_DRIVER_H

// ========================================================================
// 超声波传感器驱动头文件
// ========================================================================
// 本驱动程序实现基于超声波测距传感器（如 HC-SR04）的测距功能
// 支持最多 4 个传感器同时工作
//
// 工作原理:
//   - 触发阶段: 向 TRIG 引脚发送至少 10us 的高电平脉冲
//   - 测距阶段: 传感器自动发送 8 个 40kHz 的超声波脉冲
//   - 回波阶段: ECHO 引脚输出高电平，持续时间与距离成正比
//   - 距离计算: 距离 = (高电平时间 × 声速) / 2
//
// 硬件连接:
//   - 传感器 0: TRIG -> PB0, ECHO -> PA0
//   - 传感器 1: TRIG -> PB1, ECHO -> PA1
//   - 传感器 2: TRIG -> PB2, ECHO -> PA2
//   - 传感器 3: TRIG -> PB3, ECHO -> PA3
// ========================================================================

#include "stm32f10x.h"
#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "stm32_clock.h"

// ========================================================================
// 全局变量声明
// ========================================================================
// 记录各个传感器的测距数据
//
// record_tick_X: 记录 ECHO 引脚上升沿的时刻（回波开始时间），单位：毫秒
// distance_X: 计算得到的距离值，单位：毫米（声速 340m/s）
//
// 注意: 这些变量使用 volatile 关键字修饰，确保在中断和主循环之间正确访问
//       防止编译器优化导致的数据不一致问题
// ========================================================================

extern volatile uint32_t record_tick_0;   // 传感器 0 回波开始时间
extern volatile uint32_t distance_0;     // 传感器 0 距离值
extern volatile uint32_t record_tick_1;   // 传感器 1 回波开始时间
extern volatile uint32_t distance_1;     // 传感器 1 距离值
extern volatile uint32_t record_tick_2;   // 传感器 2 回波开始时间
extern volatile uint32_t distance_2;     // 传感器 2 距离值
extern volatile uint32_t record_tick_3;   // 传感器 3 回波开始时间
extern volatile uint32_t distance_3;     // 传感器 3 距离值

// ========================================================================
// 函数声明
// ========================================================================

/**
 * @brief 初始化超声波传感器驱动
 * @note  配置 GPIO、外部中断和 NVIC
 *        - PA0-PA3 配置为浮空输入（ECHO 引脚）
 *        - PB0-PB3 配置为推挽输出（TRIG 引脚）
 *        - 配置 EXTI0-3 外部中断，双边沿触发
 *        - 配置 NVIC 中断优先级
 */
void sensor_init();

/**
 * @brief 触发传感器 0 发送超声波
 * @note  向 PB0 (TRIG) 输出 10us 高电平脉冲
 *        触发后，传感器会自动发送超声波并输出回波信号
 */
void trig_0();

/**
 * @brief 触发传感器 1 发送超声波
 * @note  向 PB1 (TRIG) 输出 10us 高电平脉冲
 */
void trig_1();

/**
 * @brief 触发传感器 2 发送超声波
 * @note  向 PB2 (TRIG) 输出 10us 高电平脉冲
 */
void trig_2();

/**
 * @brief 触发传感器 3 发送超声波
 * @note  向 PB3 (TRIG) 输出 10us 高电平脉冲
 */
void trig_3();

#endif // __SENSOR_DRIVER_H