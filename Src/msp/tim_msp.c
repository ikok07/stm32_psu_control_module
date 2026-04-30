//
// Created by Kok on 4/30/26.
//

#include "app_state.h"
#include "stm32f4xx_hal.h"

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    HAL_StatusTypeDef hal_err = HAL_OK;
    // Set up TIM9 for HAL Ticks
    gAppState.htim9.Instance = TIM9;
    gAppState.htim9.Init.Prescaler = (SystemCoreClock / 1000000U) - 1;      // 1 MHz
    gAppState.htim9.Init.Period = (1000000U / configTICK_RATE_HZ) - 1;      // 1 ms
    gAppState.htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if ((hal_err = HAL_TIM_Base_Init(&gAppState.htim9)) != HAL_OK) return hal_err;
    if ((hal_err = HAL_TIM_Base_Start_IT(&gAppState.htim9)) != HAL_OK) return hal_err;
    return hal_err;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM9) {
        __HAL_RCC_TIM9_CLK_ENABLE();

        HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    }
}