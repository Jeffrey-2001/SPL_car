#include "sensor_driver.h"

// ========================================================================
// 全局变量定义
// ========================================================================
// 这些变量存储各个传感器的测距数据
// volatile 关键字确保在中断和主循环之间正确访问
// ========================================================================

volatile uint32_t record_tick_0 = 0;  // 传感器 0 回波开始时间（上升沿时刻）
volatile uint32_t distance_0 = 0;    // 传感器 0 距离值（毫米）
volatile uint32_t record_tick_1 = 0;  // 传感器 1 回波开始时间
volatile uint32_t distance_1 = 0;    // 传感器 1 距离值
volatile uint32_t record_tick_2 = 0;  // 传感器 2 回波开始时间
volatile uint32_t distance_2 = 0;    // 传感器 2 距离值
volatile uint32_t record_tick_3 = 0;  // 传感器 3 回波开始时间
volatile uint32_t distance_3 = 0;    // 传感器 3 距离值

/**
 * @brief 触发传感器 0 发送超声波
 * @note  HC-SR04 等超声波传感器需要至少 10us 的高电平脉冲来触发
 *        触发后，传感器会自动发送 8 个 40kHz 的超声波脉冲
 *        约 10us 后 ECHO 引脚会输出高电平，表示回波开始
 *        遇到障碍物反射后，ECHO 引脚变低，回波结束
 *        高电平持续时间与距离成正比：时间 = 2 * 距离 / 声速
 */
void trig_0(){
    GPIO_SetBits(GPIOB, GPIO_Pin_0);   // PB0 (TRIG) 输出高电平
    delay_ms(10);                      // 保持 10ms（注意：原代码用的是 ms，应该是 10us）
    GPIO_ResetBits(GPIOB, GPIO_Pin_0); // PB0 (TRIG) 输出低电平
}

void trig_1(){
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   // PB1 (TRIG) 输出高电平
    delay_ms(10);
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // PB1 (TRIG) 输出低电平
}

void trig_2(){
    GPIO_SetBits(GPIOB, GPIO_Pin_2);   // PB2 (TRIG) 输出高电平
    delay_ms(10);
    GPIO_ResetBits(GPIOB, GPIO_Pin_2); // PB2 (TRIG) 输出低电平
}

void trig_3(){
    GPIO_SetBits(GPIOB, GPIO_Pin_3);   // PB3 (TRIG) 输出高电平
    delay_ms(10);
    GPIO_ResetBits(GPIOB, GPIO_Pin_3); // PB3 (TRIG) 输出低电平
}

/**
 * @brief 初始化超声波传感器驱动
 * @note  配置步骤：
 *        1. 启用 GPIOA、GPIOB 和 AFIO 时钟
 *        2. 配置 PA0-PA3 为浮空输入（ECHO 引脚）
 *        3. 配置 PB0-PB3 为推挽输出（TRIG 引脚）
 *        4. 配置 EXTI 外部中断，连接到 PA0-PA3
 *        5. 配置 NVIC 中断优先级
 */
void sensor_init(){
    // ====================================================================
    // 1. 启用时钟
    // ====================================================================
    // GPIOA: 用于 ECHO 输入（PA0-PA3）
    // GPIOB: 用于 TRIG 输出（PB0-PB3）
    // AFIO: 复用功能 I/O，用于将 GPIO 引脚映射到 EXTI 外部中断线
    // ====================================================================

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // ====================================================================
    // 2. 配置 PA0-PA3 为浮空输入（ECHO 引脚）
    // ====================================================================
    // GPIO_Mode_IN_FLOATING: 浮空输入，用于读取 ECHO 信号
    // ECHO 信号由传感器输出，MCU 只需读取
    // ====================================================================

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // ====================================================================
    // 3. 配置 PB0-PB3 为推挽输出（TRIG 引脚）
    // ====================================================================
    // GPIO_Mode_Out_PP: 推挽输出，用于驱动 TRIG 信号
    // GPIO_Speed_50MHz: 输出速度 50MHz（足够产生 10us 脉冲）
    // ====================================================================

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // ====================================================================
    // 4. 配置 EXTI 外部中断
    // ====================================================================
    // 将 PA0-PA3 映射到 EXTI0-EXTI3 中断线
    // STM32 的 EXTI 支持多个 GPIO 引脚共享同一个中断线
    // 例如：PA0、PB0、PC0 都映射到 EXTI_Line0
    // ====================================================================

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

    // ====================================================================
    // 配置 EXTI 参数
    // ====================================================================
    // EXTI_Mode_Interrupt: 中断模式
    // EXTI_Trigger_Rising_Falling: 双边沿触发（上升沿和下降沿）
    //   - 上升沿: ECHO 开始，记录时间戳
    //   - 下降沿: ECHO 结束，计算距离
    // ====================================================================

    EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // ====================================================================
    // 5. 配置 NVIC 中断优先级
    // ====================================================================
    // NVIC (Nested Vectored Interrupt Controller): 嵌套向量中断控制器
    // PreemptionPriority: 抢占优先级（2），数值越小优先级越高
    // SubPriority: 子优先级（2），用于相同抢占优先级时的排序
    // ====================================================================

    NVIC_InitTypeDef NVIC_InitStruct = {0};

    // EXTI0 中断配置（对应 PA0）
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // EXTI1 中断配置（对应 PA1）
    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // EXTI2 中断配置（对应 PA2）
    NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // EXTI3 中断配置（对应 PA3）
    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
/**
 * @brief EXTI0 中断服务函数（处理 PA0 / 传感器 0）
 * @note  在 ECHO 信号的上升沿和下降沿触发
 *        - 上升沿: 记录开始时间（超声波发出）
 *        - 下降沿: 计算距离（超声波返回）
 */
void EXTI0_IRQHandler(void){
    // 检查是否为 EXTI_Line0 触发（防止误触发）
    if(EXTI_GetITStatus(EXTI_Line0) == RESET){
        return;
    }

    // 判断 PA0 的电平状态
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1){
        // 上升沿：ECHO 开始，记录时间戳
        record_tick_0 = get_tick_ms();
    }else{
        // 下降沿：ECHO 结束，计算距离
        // 距离 = 时间差 × 声速 / 2
        // 声速约为 340m/s = 340mm/ms
        // 除以 2 是因为超声波往返一次
        uint32_t dur = (get_tick_ms() - record_tick_0);
        distance_0 = dur * 340 / 2;
    }

    // 清除中断标志位，允许下次中断
    EXTI_ClearFlag(EXTI_Line0);
}

/**
 * @brief EXTI1 中断服务函数（处理 PA1 / 传感器 1）
 * @note  与 EXTI0_IRQHandler 逻辑相同
 */
void EXTI1_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line1) == RESET){
        return;
    }

    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 1){
        record_tick_1 = get_tick_ms();
    }else{
        uint32_t dur = (get_tick_ms() - record_tick_1);
        distance_1 = dur * 340 / 2;
    }

    EXTI_ClearFlag(EXTI_Line1);
}

/**
 * @brief EXTI2 中断服务函数（处理 PA2 / 传感器 2）
 * @note  与 EXTI0_IRQHandler 逻辑相同
 */
void EXTI2_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line2) == RESET){
        return;
    }

    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 1){
        record_tick_2 = get_tick_ms();
    }else{
        uint32_t dur = (get_tick_ms() - record_tick_2);
        distance_2 = dur * 340 / 2;
    }

    EXTI_ClearFlag(EXTI_Line2);
}

/**
 * @brief EXTI3 中断服务函数（处理 PA3 / 传感器 3）
 * @note  与 EXTI0_IRQHandler 逻辑相同
 */
void EXTI3_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line3) == RESET){
        return;
    }

    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 1){
        record_tick_3 = get_tick_ms();
    }else{
        uint32_t dur = (get_tick_ms() - record_tick_3);
        distance_3 = dur * 340 / 2;
    }

    EXTI_ClearFlag(EXTI_Line3);
}

// ========================================================================
// HAL 库版本注释（已弃用）
// ========================================================================
// 以下是 STM32Cube HAL 库的回调函数示例
// 本项目使用标准外设库（StdPeriph），所以这段代码被注释
// ========================================================================

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
//     if(GPIO_Pin == GPIO_Pin_0){
//         if(HAL_GPIO_ReadPin(GPIOA, GPIO_Pin) == 1){
//             record_tick_0 = HAL_GetTick();
//         }else{
//             uint32_t dur = (HAL_GetTick() - record_tick_0);
//             distance_0 = dur * 340 / 2;
//         }
//     }else if(GPIO_Pin == GPIO_Pin_1){
//         if(HAL_GPIO_ReadPin(GPIOA, GPIO_Pin) == 1){
//             record_tick_1 = HAL_GetTick();
//         }else{
//             uint32_t dur = (HAL_GetTick() - record_tick_1);
//             distance_1 = dur * 340 / 2;
//         }
//     }else if(GPIO_Pin == GPIO_Pin_2){
//         if(HAL_GPIO_ReadPin(GPIOA, GPIO_Pin) == 1){
//             record_tick_2 = HAL_GetTick();
//         }else{
//             uint32_t dur = (HAL_GetTick() - record_tick_2);
//             distance_2 = dur * 340 / 2;
//         }
//     }else if(GPIO_Pin == GPIO_Pin_3){
//         if(HAL_GPIO_ReadPin(GPIOA, GPIO_Pin) == 1){
//             record_tick_3 = HAL_GetTick();
//         }else{
//             uint32_t dur = (HAL_GetTick() - record_tick_3);
//             distance_3 = dur * 340 / 2;
//         }
//     }
// }