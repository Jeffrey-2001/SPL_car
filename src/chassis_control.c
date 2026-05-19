#include "chassis_control.h"
#include "sensor_driver.h"
#include "motor_driver.h"
#include "stm32_clock.h"
#include <stddef.h>

// ========================================================================
// 全局变量
// ========================================================================

static chassis_state_t current_state = CHASSIS_STATE_STOP;
static chassis_mode_t current_mode = CHASSIS_MODE_AUTO;  // 默认自动模式
static chassis_config_t config = CHASSIS_DEFAULT_CONFIG;
static uint32_t state_timer = 0;  // 状态计时器（单位：调用周期）

// 传感器数据缓存（避免直接访问 volatile 变量）
static uint32_t sensor_dist[4] = {0};

// ========================================================================
// 内部辅助函数
// ========================================================================

/**
 * @brief 读取所有传感器数据到缓存
 * @note  使用临时缓存避免在状态计算过程中读取到不一致的数据
 */
static void read_sensors(void) {
    sensor_dist[0] = distance_0;
    sensor_dist[1] = distance_1;
    sensor_dist[2] = distance_2;
    sensor_dist[3] = distance_3;
}

/**
 * @brief 检测前方是否有障碍物
 * @return true: 前方有障碍物, false: 前方无障碍物
 */
static bool check_front_blocked(void) {
    // 前方: 传感器0 (前左) 和 传感器1 (前右)
    // 任意一个传感器检测到障碍物即认为前方受阻
    return (sensor_dist[0] < config.stop_distance && sensor_dist[0] > 0) ||
           (sensor_dist[1] < config.stop_distance && sensor_dist[1] > 0);
}

/**
 * @brief 检测后方是否有障碍物
 * @return true: 后方有障碍物, false: 后方无障碍物
 */
static bool check_back_blocked(void) {
    // 后方: 传感器2 (后左) 和 传感器3 (后右)
    return (sensor_dist[2] < config.stop_distance && sensor_dist[2] > 0) ||
           (sensor_dist[3] < config.stop_distance && sensor_dist[3] > 0);
}

/**
 * @brief 检测左侧是否有障碍物
 * @return true: 左侧有障碍物, false: 左侧无障碍物
 */
static bool check_left_blocked(void) {
    // 左侧: 传感器0 (前左) 和 传感器2 (后左)
    // 使用较小的危险距离阈值
    return (sensor_dist[0] < config.danger_distance && sensor_dist[0] > 0) ||
           (sensor_dist[2] < config.danger_distance && sensor_dist[2] > 0);
}

/**
 * @brief 检测右侧是否有障碍物
 * @return true: 右侧有障碍物, false: 右侧无障碍物
 */
static bool check_right_blocked(void) {
    // 右侧: 传感器1 (前右) 和 传感器3 (后右)
    return (sensor_dist[1] < config.danger_distance && sensor_dist[1] > 0) ||
           (sensor_dist[3] < config.danger_distance && sensor_dist[3] > 0);
}

/**
 * @brief 获取左侧最远距离（用于判断转向空间）
 * @return 左侧传感器的最大距离值
 */
static uint32_t get_left_distance(void) {
    uint32_t d0 = (sensor_dist[0] > 0) ? sensor_dist[0] : 9999;
    uint32_t d2 = (sensor_dist[2] > 0) ? sensor_dist[2] : 9999;
    return (d0 > d2) ? d0 : d2;
}

/**
 * @brief 获取右侧最远距离（用于判断转向空间）
 * @return 右侧传感器的最大距离值
 */
static uint32_t get_right_distance(void) {
    uint32_t d1 = (sensor_dist[1] > 0) ? sensor_dist[1] : 9999;
    uint32_t d3 = (sensor_dist[3] > 0) ? sensor_dist[3] : 9999;
    return (d1 > d3) ? d1 : d3;
}

/**
 * @brief 执行当前状态的运动控制
 */
static void execute_motion(void) {
    switch (current_state) {
        case CHASSIS_STATE_FORWARD:
            set_speed(config.left_motor_id, config.forward_speed);
            set_speed(config.right_motor_id, config.forward_speed);
            break;

        case CHASSIS_STATE_BACKWARD:
            set_speed(config.left_motor_id, -config.backward_speed);
            set_speed(config.right_motor_id, -config.backward_speed);
            break;

        case CHASSIS_STATE_TURN_LEFT:
            turn_left(config.left_motor_id, config.right_motor_id, config.turn_speed);
            break;

        case CHASSIS_STATE_TURN_RIGHT:
            turn_right(config.left_motor_id, config.right_motor_id, config.turn_speed);
            break;

        case CHASSIS_STATE_STOP:
            stop_emergency(config.left_motor_id);
            stop_emergency(config.right_motor_id);
            break;

        default:
            stop_emergency(config.left_motor_id);
            stop_emergency(config.right_motor_id);
            break;
    }
}

/**
 * @brief 状态转换逻辑（核心避障算法）
 */
static void update_state_machine(void) {
    bool front_blocked = check_front_blocked();
    bool back_blocked = check_back_blocked();
    bool left_blocked = check_left_blocked();
    bool right_blocked = check_right_blocked();

    switch (current_state) {
        case CHASSIS_STATE_FORWARD:
            // 前进过程中检测到障碍物
            if (front_blocked) {
                current_state = CHASSIS_STATE_STOP;
                state_timer = 0;
            }
            break;

        case CHASSIS_STATE_STOP:
            state_timer++;
            // 停止一小段时间后决定下一步动作
            if (state_timer > 2) {
                if (front_blocked && !back_blocked) {
                    // 前方有障碍物，后方无障碍物，开始后退
                    current_state = CHASSIS_STATE_BACKWARD;
                    state_timer = 0;
                } else if (!front_blocked) {
                    // 前方无障碍物，恢复前进
                    current_state = CHASSIS_STATE_FORWARD;
                    state_timer = 0;
                }
                // 如果前后都有障碍物，保持停止状态
            }
            break;

        case CHASSIS_STATE_BACKWARD:
            state_timer++;
            // 后退一段时间后开始转向
            uint32_t back_ticks = config.backward_time_ms / 50;  // 假设50ms调用周期
            if (state_timer > back_ticks) {
                uint32_t left_dist = get_left_distance();
                uint32_t right_dist = get_right_distance();

                if (right_dist > left_dist && !right_blocked) {
                    // 右侧空间更大，右转
                    current_state = CHASSIS_STATE_TURN_RIGHT;
                } else if (!left_blocked) {
                    // 左侧有空间，左转
                    current_state = CHASSIS_STATE_TURN_LEFT;
                } else if (!front_blocked) {
                    // 无法转向，但前方已空，恢复前进
                    current_state = CHASSIS_STATE_FORWARD;
                } else {
                    // 无法转向，前方也有障碍物，停止
                    current_state = CHASSIS_STATE_STOP;
                }
                state_timer = 0;
            }
            // 后退过程中检测到后方障碍物
            if (back_blocked) {
                current_state = CHASSIS_STATE_STOP;
                state_timer = 0;
            }
            break;

        case CHASSIS_STATE_TURN_LEFT:
        case CHASSIS_STATE_TURN_RIGHT:
            state_timer++;
            // 转向一段时间后恢复前进
            uint32_t turn_ticks = config.turn_time_ms / 50;
            if (state_timer > turn_ticks) {
                if (!front_blocked) {
                    current_state = CHASSIS_STATE_FORWARD;
                } else {
                    current_state = CHASSIS_STATE_STOP;
                }
                state_timer = 0;
            }
            break;

        default:
            current_state = CHASSIS_STATE_STOP;
            break;
    }
}

// ========================================================================
// 公共接口函数实现
// ========================================================================

void chassis_init(void) {
    current_state = CHASSIS_STATE_STOP;
    state_timer = 0;
    // 初始状态为停止，等待状态机接管
}

void chassis_set_config(chassis_config_t* cfg) {
    if (cfg != NULL) {
        config = *cfg;
    }
}

chassis_state_t chassis_get_state(void) {
    return current_state;
}

const char* chassis_state_name(chassis_state_t state) {
    switch (state) {
        case CHASSIS_STATE_FORWARD:  return "FORWARD";
        case CHASSIS_STATE_BACKWARD: return "BACKWARD";
        case CHASSIS_STATE_TURN_LEFT: return "TURN_LEFT";
        case CHASSIS_STATE_TURN_RIGHT: return "TURN_RIGHT";
        case CHASSIS_STATE_STOP:     return "STOP";
        default:                     return "UNKNOWN";
    }
}

void chassis_trigger_sensors(void) {
    trig_0();
    delay_ms(20);
    trig_1();
    delay_ms(20);
    trig_2();
    delay_ms(20);
    trig_3();
    delay_ms(20);
}

void chassis_update(void) {
    // 根据模式选择不同的控制逻辑
    if (current_mode == CHASSIS_MODE_AUTO) {
        // 自动避障模式
        // 1. 触发传感器测距
        chassis_trigger_sensors();

        // 2. 读取传感器数据
        read_sensors();

        // 3. 更新状态机
        update_state_machine();

        // 4. 执行运动控制
        execute_motion();
    } else {
        // 遥控模式
        // 运动控制由remote模块调用chassis_set_state控制
        // 这里只执行运动
        execute_motion();
    }
}

void chassis_set_state(chassis_state_t state) {
    current_state = state;
    state_timer = 0;
}

void chassis_emergency_stop(void) {
    current_state = CHASSIS_STATE_STOP;
    stop_emergency(config.left_motor_id);
    stop_emergency(config.right_motor_id);
}

// ========================================================================
// 模式控制函数实现
// ========================================================================

chassis_mode_t chassis_get_mode(void) {
    return current_mode;
}

void chassis_set_mode(chassis_mode_t mode) {
    if (current_mode != mode) {
        current_mode = mode;
        // 切换模式时先停止
        chassis_emergency_stop();
        state_timer = 0;
    }
}

void chassis_toggle_mode(void) {
    if (current_mode == CHASSIS_MODE_AUTO) {
        chassis_set_mode(CHASSIS_MODE_REMOTE);
    } else {
        chassis_set_mode(CHASSIS_MODE_AUTO);
    }
}
