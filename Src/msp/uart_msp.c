//
// Created by Kok on 4/9/26.
//

#include "stm32f4xx_hal.h"
#include "gpio_defs.h"

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_AF_PP,
        .Pin = UART_TX_GPIO_PIN,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FAST,
        .Alternate = GPIO_AF7_USART1
    };

    HAL_GPIO_Init(UART_TX_GPIO_PORT, &GPIO_Config);
}