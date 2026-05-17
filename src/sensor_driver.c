#include "sensor_driver.h"

volatile uint32_t record_tick_0 = 0;
volatile uint32_t distance_0 = 0;
volatile uint32_t record_tick_1 = 0;
volatile uint32_t distance_1 = 0;
volatile uint32_t record_tick_2 = 0;
volatile uint32_t distance_2 = 0;
volatile uint32_t record_tick_3 = 0;
volatile uint32_t distance_3 = 0;

void trig_0(){
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    delay_ms(10);
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

void sensor_init(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

    EXTI_InitTypeDef EXTI_InitStruct = {0};
    EXTI_InitStruct.EXTI_Line = EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct = {0};
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void EXTI0_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line0) == RESET){
        return;
    }

    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1){
        record_tick_0 = get_tick_ms();
    }else{
        uint32_t dur = (get_tick_ms() - record_tick_0);
        distance_0 = dur * 340 / 2;
    }

    EXTI_ClearFlag(EXTI_Line0);
}

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