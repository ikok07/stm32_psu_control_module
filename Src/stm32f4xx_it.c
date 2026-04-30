//
// Created by Kok on 4/14/26.
//

#include "app_state.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "stm32f4xx_hal.h"
#include "task.h"

void TIM1_BRK_TIM9_IRQHandler() {
    __HAL_TIM_CLEAR_IT(&gAppState.htim9, TIM_IT_UPDATE);
    HAL_IncTick();
    HAL_TIM_IRQHandler(&gAppState.htim9);
}

void EXTI0_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}
