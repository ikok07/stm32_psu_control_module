//
// Created by Kok on 4/14/26.
//

#include "stm32f4xx_hal.h"

void vApplicationTickHook() {
    HAL_IncTick();
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
