#include "stm32_watchdog.h"

// 看门狗
void STM_Watchdog_Init(void)
{				
	IWDG->KR = (uint16_t)0xAAAA;// 允许访问寄存器
	
	IWDG->PR = (uint16_t)0x0003;// 32分频,最大实现3276.8ms
	IWDG->RLR = 0xFFF;// 重载值,设定为3276.8ms
	
	IWDG->KR = (uint16_t)0xCCCC;// 启动看门狗
}
// 喂看门狗
void STM_Watchdog_Feed(void)
{
	IWDG->KR = (uint16_t)0xAAAA;
}




