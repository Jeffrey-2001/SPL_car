#include "remote_control.h"
#include "ps2_driver.h"

// 全局变量
static JOYSTICK_TypeDef ps2_data = {0};
static chassis_state_t remote_state = CHASSIS_STATE_STOP;
static int16_t remote_speed = 0;
static int16_t remote_turn_speed = 0;
static bool emergency_stop = false;
static uint8_t speed_level = 3;  // 默认速度档位1-5
static uint16_t speed_table[5] = {200, 400, 600, 800, 1000};  // 速度档位表

/**
 * @brief 遥控初始化
 */
void remote_init(void)
{
    ps2_init();

    // 初始化参数
    remote_state = CHASSIS_STATE_STOP;
    remote_speed = 0;
    remote_turn_speed = 0;
    emergency_stop = false;
    speed_level = 3;
}

/**
 * @brief 检测摇杆是否在中间位置（死区检测）
 * @param value 摇杆值 (0-255)
 * @return true: 在死区, false: 超出死区
 */
static bool is_in_dead_zone(uint8_t value)
{
    // 死区范围：120-135之间认为是中间
    return (value >= 120 && value <= 135);
}

/**
 * @brief 遥控更新
 */
void remote_update(void)
{
    // 扫描手柄数据
    ps2_scan(&ps2_data);

    // 检测紧急停止（L2或R2）
    if ((ps2_data.btn2 & PS2_BTN_L2) || (ps2_data.btn2 & PS2_BTN_R2)) {
        emergency_stop = true;
        remote_state = CHASSIS_STATE_STOP;
        remote_speed = 0;
        remote_turn_speed = 0;
        return;
    } else {
        emergency_stop = false;
    }

    // 检测速度调节（L1/R1）
    static bool l1_pressed = false;
    static bool r1_pressed = false;

    if ((ps2_data.btn2 & PS2_BTN_L1) && !l1_pressed) {
        l1_pressed = true;
        if (speed_level > 1) speed_level--;
    } else if (!(ps2_data.btn2 & PS2_BTN_L1)) {
        l1_pressed = false;
    }

    if ((ps2_data.btn2 & PS2_BTN_R1) && !r1_pressed) {
        r1_pressed = true;
        if (speed_level < 5) speed_level++;
    } else if (!(ps2_data.btn2 & PS2_BTN_R1)) {
        r1_pressed = false;
    }

    // 处理手柄输入
    // 优先级：方向键 > 左摇杆

    // 1. 检测方向键
    bool up_pressed = (ps2_data.btn1 & PS2_BTN_UP) != 0;
    bool down_pressed = (ps2_data.btn1 & PS2_BTN_DOWN) != 0;
    bool left_pressed = (ps2_data.btn1 & PS2_BTN_LEFT) != 0;
    bool right_pressed = (ps2_data.btn1 & PS2_BTN_RIGHT) != 0;

    if (up_pressed || down_pressed || left_pressed || right_pressed) {
        // 方向键控制
        if (up_pressed && !down_pressed) {
            remote_state = CHASSIS_STATE_FORWARD;
            remote_speed = speed_table[speed_level - 1];
            remote_turn_speed = 0;
        } else if (down_pressed && !up_pressed) {
            remote_state = CHASSIS_STATE_BACKWARD;
            remote_speed = -speed_table[speed_level - 1];
            remote_turn_speed = 0;
        } else if (left_pressed && !right_pressed) {
            remote_state = CHASSIS_STATE_TURN_LEFT;
            remote_speed = 0;
            remote_turn_speed = speed_table[speed_level - 1] / 2;
        } else if (right_pressed && !left_pressed) {
            remote_state = CHASSIS_STATE_TURN_RIGHT;
            remote_speed = 0;
            remote_turn_speed = speed_table[speed_level - 1] / 2;
        } else {
            // 同时按下相反键，停止
            remote_state = CHASSIS_STATE_STOP;
            remote_speed = 0;
            remote_turn_speed = 0;
        }
        return;
    }

    // 2. 检测左摇杆控制
    bool joy_up = !is_in_dead_zone(ps2_data.LJoy_UD) && (ps2_data.LJoy_UD < 120);
    bool joy_down = !is_in_dead_zone(ps2_data.LJoy_UD) && (ps2_data.LJoy_UD > 135);
    bool joy_left = !is_in_dead_zone(ps2_data.LJoy_LR) && (ps2_data.LJoy_LR < 120);
    bool joy_right = !is_in_dead_zone(ps2_data.LJoy_LR) && (ps2_data.LJoy_LR > 135);

    if (joy_up || joy_down || joy_left || joy_right) {
        // 摇杆控制（模拟量）
        // 计算速度（基于摇杆偏移量）
        int16_t ud_offset = (int16_t)ps2_data.LJoy_UD - 128;  // -128 ~ 127
        int16_t lr_offset = (int16_t)ps2_data.LJoy_LR - 128;  // -128 ~ 127

        // 归一化到速度范围
        int16_t max_speed = speed_table[speed_level - 1];
        remote_speed = (ud_offset * max_speed) / 80;  // 80是有效范围的半值
        remote_turn_speed = (lr_offset * max_speed) / 80;

        // 确定状态
        if (remote_speed > 50) {
            remote_state = CHASSIS_STATE_FORWARD;
        } else if (remote_speed < -50) {
            remote_state = CHASSIS_STATE_BACKWARD;
        } else {
            // 速度接近0，只转向
            remote_speed = 0;
            if (remote_turn_speed > 30) {
                remote_state = CHASSIS_STATE_TURN_RIGHT;
            } else if (remote_turn_speed < -30) {
                remote_state = CHASSIS_STATE_TURN_LEFT;
            } else {
                remote_state = CHASSIS_STATE_STOP;
            }
        }
    } else {
        // 没有输入，停止
        remote_state = CHASSIS_STATE_STOP;
        remote_speed = 0;
        remote_turn_speed = 0;
    }
}

/**
 * @brief 获取手柄期望的底盘状态
 */
chassis_state_t remote_get_state(void)
{
    return remote_state;
}

/**
 * @brief 获取手柄期望的速度值
 */
int16_t remote_get_speed(void)
{
    return remote_speed;
}

/**
 * @brief 获取手柄期望的转向速度值
 */
int16_t remote_get_turn_speed(void)
{
    return remote_turn_speed;
}

/**
 * @brief 获取是否按下紧急停止按钮
 */
bool remote_is_emergency_stop(void)
{
    return emergency_stop;
}

/**
 * @brief 获取当前遥控速度档位
 */
uint8_t remote_get_speed_level(void)
{
    return speed_level;
}

/**
 * @brief 检测是否按下SELECT按键
 */
bool remote_is_select_pressed(void)
{
    return (ps2_data.btn1 & PS2_BTN_SELECT) != 0;
}

/**
 * @brief 获取PS2手柄原始数据
 */
const JOYSTICK_TypeDef* remote_get_ps2_data(void)
{
    return &ps2_data;
}
