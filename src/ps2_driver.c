#include "ps2_driver.h"
#include "stm32_clock.h"

// PS2手柄引脚定义
#define PS2_DI()     ((GPIOA->IDR & GPIO_Pin_4) != 0)  // 数据读取（来自手柄）

#define PS2_CMD_H()  (GPIOA->BSRR = GPIO_Pin_5)      // 命令位高
#define PS2_CMD_L()  (GPIOA->BRR = GPIO_Pin_5)       // 命令位低

#define PS2_CS_H()   (GPIOA->BSRR = GPIO_Pin_6)      // CS拉高
#define PS2_CS_L()   (GPIOA->BRR = GPIO_Pin_6)       // CS拉低

#define PS2_CLK_H()  (GPIOA->BSRR = GPIO_Pin_7)      // 时钟拉高
#define PS2_CLK_L()  (GPIOA->BRR = GPIO_Pin_7)       // 时钟拉低

const uint8_t PS2_cmnd[9] = {0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t PS2_data[9] = {0};

/**
 * @brief PS2初始化
 */
void ps2_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 启用GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // DI (PA4) 数据输入（来自手柄）上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CMD (PA5) 命令输出（到手柄）推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CS (PA6) 片选信号（通讯期间处于低电平）推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CLK (PA7) 时钟信号（到手柄）推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 关闭PS2手柄使用
    PS2_CS_H();
}

/**
 * @brief PS2数据读写函数
 * @param cmd 要写入的命令
 * @return 读取的数据
 */
static uint8_t ps2_read_write(uint8_t cmd)
{
    volatile uint8_t res = 0;
    volatile uint8_t ref;

    for(ref = 0x01; ref > 0x00; ref <<= 1)
    {
        // 发送第一位数据
        if(ref & cmd)
            PS2_CMD_H();
        else
            PS2_CMD_L();

        PS2_CLK_L();
        ps2_delay_us(16);

        // 读取一位数据
        if(PS2_DI())
            res |= ref;
        PS2_CLK_H();
        ps2_delay_us(16);
    }

    return res;
}

/**
 * @brief PS2读取按键和摇杆值
 * @param JoystickStruct 手柄值结构体指针
 */
void ps2_scan(JOYSTICK_TypeDef* JoystickStruct)
{
    uint8_t i;

    // 使能手柄
    PS2_CS_L();

    // 读取PS2数据
    for(i = 0; i < 9; i++)
    {
        PS2_data[i] = ps2_read_write(PS2_cmnd[i]);
    }

    // 关闭使能
    PS2_CS_H();

    // 赋值
    JoystickStruct->mode = PS2_data[1];
    JoystickStruct->btn1 = ~PS2_data[3];
    JoystickStruct->btn2 = ~PS2_data[4];
    JoystickStruct->RJoy_LR = PS2_data[5];
    JoystickStruct->RJoy_UD = PS2_data[6];
    JoystickStruct->LJoy_LR = PS2_data[7];
    JoystickStruct->LJoy_UD = PS2_data[8];
}

/**
 * @brief 延时微秒
 * @param us 延时时间(微秒)
 * @note  使用SysTick实现微秒级延时
 */
void ps2_delay_us(uint32_t us)
{
    // 假设系统时钟72MHz，每个tick约为1/72us
    // 使用简单的循环延时
    volatile uint32_t count;
    for(count = 0; count < us * 6; count++) {
        __NOP();
    }
}
