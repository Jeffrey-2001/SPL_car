#ifndef __CHASSIS_CONTROL_H
#define __CHASSIS_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x.h"

// ========================================================================
// 底盘状态机
// ========================================================================
// 实现基于超声波传感器的自动避障功能
//
// 传感器布局假设:
//   传感器0 (PA0/PB0): 前左
//   传感器1 (PA1/PB1): 前右
//   传感器2 (PA2/PB2): 后左
//   传感器3 (PA3/PB3): 后右
//
// 避障逻辑:
//   - 前方有障碍物: 停止 -> 后退 -> 转向
//   - 后方有障碍物: 停止 -> 前进 -> 转向
//   - 转向方向选择: 优先选择障碍物较远的一侧
// ========================================================================

// 底盘状态枚举
typedef enum {
    CHASSIS_STATE_FORWARD,     // 前进
    CHASSIS_STATE_BACKWARD,    // 后退
    CHASSIS_STATE_TURN_LEFT,   // 原地左转
    CHASSIS_STATE_TURN_RIGHT,  // 原地右转
    CHASSIS_STATE_STOP         // 紧急停止
} chassis_state_t;

// 底盘控制模式枚举
typedef enum {
    CHASSIS_MODE_AUTO,      // 自动避障模式
    CHASSIS_MODE_REMOTE,    // 遥控模式
} chassis_mode_t;

// 避障参数配置
typedef struct {
    uint16_t forward_speed;       // 前进速度 (0-32767)
    uint16_t backward_speed;      // 后退速度 (0-32767)
    uint16_t turn_speed;          // 转向速度 (0-32767)
    uint16_t stop_distance;       // 停止距离阈值 (毫米)
    uint16_t danger_distance;     // 危险距离阈值 (毫米)
    uint16_t backward_time_ms;    // 后退持续时间
    uint16_t turn_time_ms;        // 转向持续时间
    uint8_t  left_motor_id;       // 左电机ID
    uint8_t  right_motor_id;      // 右电机ID
} chassis_config_t;

// 默认配置
#define CHASSIS_DEFAULT_CONFIG {     \
    .forward_speed = 500,            \
    .backward_speed = 300,           \
    .turn_speed = 300,               \
    .stop_distance = 500,            \
    .danger_distance = 200,          \
    .backward_time_ms = 1000,        \
    .turn_time_ms = 500,             \
    .left_motor_id = 0x01,           \
    .right_motor_id = 0x02,          \
}

// ========================================================================
// 函数声明
// ========================================================================

/**
 * @brief 初始化底盘控制
 * @note  设置默认配置和初始状态
 */
void chassis_init(void);

/**
 * @brief 获取当前底盘控制模式
 * @return 控制模式枚举值
 */
chassis_mode_t chassis_get_mode(void);

/**
 * @brief 设置底盘控制模式
 * @param mode 目标控制模式
 */
void chassis_set_mode(chassis_mode_t mode);

/**
 * @brief 切换底盘控制模式
 * @note  在自动和遥控模式之间切换
 */
void chassis_toggle_mode(void);

/**
 * @brief 设置底盘配置参数
 * @param config 配置参数结构体指针
 */
void chassis_set_config(chassis_config_t* config);

/**
 * @brief 获取当前底盘状态
 * @return 当前状态枚举值
 */
chassis_state_t chassis_get_state(void);

/**
 * @brief 获取状态名称（用于调试）
 * @param state 状态枚举值
 * @return 状态名称字符串
 */
const char* chassis_state_name(chassis_state_t state);

/**
 * @brief 触发所有超声波传感器测距
 * @note  依次触发 4 个传感器，每个之间间隔 20ms 避免干扰
 */
void chassis_trigger_sensors(void);

/**
 * @brief 底盘状态机更新（主循环调用）
 * @note  根据传感器数据更新状态，并执行相应动作
 *        建议调用周期: 50-100ms
 */
void chassis_update(void);

/**
 * @brief 手动设置底盘状态（用于调试或遥控模式）
 * @param state 目标状态
 */
void chassis_set_state(chassis_state_t state);

/**
 * @brief 紧急停止
 */
void chassis_emergency_stop(void);

#endif // __CHASSIS_CONTROL_H
