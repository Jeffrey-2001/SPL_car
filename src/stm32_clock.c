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

void systick_init(uint32_t sysclk_mhz)
{
    SysTick->CTRL = 0;
    SysTick->LOAD = (sysclk_mhz * 1000) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07;
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

uint32_t RCC_Clock_Init(void)
{
    uint32_t sysclk_mhz;
    uint32_t timeout;

    RCC_DeInit();

    /* 尝试启动 HSE (8MHz)，最多等待 100ms */
    RCC_HSEConfig(RCC_HSE_ON);
    timeout = 100000;
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET && --timeout);

    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    if (timeout > 0) {
        /* HSE 正常 → 72MHz */
        FLASH_SetLatency(FLASH_Latency_2);
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        sysclk_mhz = 72;
    } else {
        /* HSE 失败 → 回退到 HSI: 8MHz/2 × 16 = 64MHz */
        FLASH_SetLatency(FLASH_Latency_1);
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
        sysclk_mhz = 64;
    }

    RCC_PLLCmd(ENABLE);
    timeout = 100000;
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET && --timeout);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08);

    return sysclk_mhz;
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








