#include "stm32_clock.h"

static uint32_t sys_tick_ms = 0;

void delay_ms(uint16_t nms)
{
    uint32_t start_tick = sys_tick_ms;
    while (sys_tick_ms - start_tick < nms);
}

// SysTick 中断服务函数（每1ms进一次）
void SysTick_Handler(void)
{
    sys_tick_ms++;
}

uint32_t get_tick_ms()
{
    return sys_tick_ms;
}

void systick_init()
{
    // 1. 先关闭 SysTick
    SysTick->CTRL = 0;
    
    // 2. 设置重装载值：72MHz / 1000 = 72000 → 减1
    SysTick->LOAD = 72000 - 1; 
    
    // 3. 清空当前计数值
    SysTick->VAL = 0;
    
    // 4. 配置：
    // bit0=1 使能
    // bit1=1 开启中断
    // bit2=1 时钟源=HCLK(72MHz)
    SysTick->CTRL = 0x07; 
    
    // 配置中断优先级（内核中断）
    NVIC_SetPriority(SysTick_IRQn, 0);
}

static uint16_t fac_us = 0;// us延时倍乘数
static uint16_t fac_ms = 0;// ms延时倍乘数

// 系统时钟(单位：M)
void STM_Clock_Init(uint16_t systick)
{
    SysTick->CTRL &= 0xfffffffb; // HCLK/8

    fac_us = systick * 1000000 / 8;
    fac_ms = fac_us / 1000;        
}

void RCC_Clock_Init(void)
{
	// 1. 复位时钟
	RCC_DeInit();
	
	// 2. 打开外部高速晶振 HSE (8MHz)
	RCC_HSEConfig(RCC_HSE_ON);
	// 等待外部晶振稳定
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

	// 3. 配置Flash（必须配置，否则72MHz会跑飞）
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);

	// 4. 总线分频配置
	RCC_HCLKConfig(RCC_SYSCLK_Div1);    // HCLK = SYSCLK = 72M
	RCC_PCLK1Config(RCC_HCLK_Div2);     // APB1 = 36M (最高只能36M)
	RCC_PCLK2Config(RCC_HCLK_Div1);     // APB2 = 72M

	// 5. PLL配置：HSE不分频 × 9倍频 = 8×9=72MHz
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	
	// 6. 开启PLL倍频
	RCC_PLLCmd(ENABLE);
	// 等待PLL锁定
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	// 7. 把PLL作为系统时钟
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	// 等待切换成功
	while (RCC_GetSYSCLKSource() != 0x08);
}

// 最简单的毫秒级阻塞延时
void STM_Delayms(uint16_t nms)
{
    uint32_t i, j;
    for(i = 0; i < nms; i++)
        for(j = 0; j < 7200; j++); // 72MHz 下 1ms 标准循环值
}

// 最简单的微秒级阻塞延时
void STM_Delayus(uint32_t nus)
{
    uint32_t i, j;
    for(i = 0; i < nus; i++)
        for(j = 0; j < 7; j++);   // 72MHz 下 1us 标准循环值
}








