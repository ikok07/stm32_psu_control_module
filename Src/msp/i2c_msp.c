//
// Created by Kok on 4/30/26.
//

#include "gpio_defs.h"
#include "stm32f4xx_hal.h"

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_AF_OD,
        .Pull = GPIO_PULLUP,                        // Physical pull-up resistor
        .Speed = GPIO_SPEED_FREQ_HIGH
    };

    __HAL_RCC_GPIOB_CLK_ENABLE();

    if (hi2c->Instance == I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();

        GPIO_Config.Alternate = GPIO_AF4_I2C1;

        GPIO_Config.Pin = DISPLAY_I2C_SDA_GPIO_PIN;
        HAL_GPIO_Init(DISPLAY_I2C_SDA_GPIO_PORT, &GPIO_Config);

        GPIO_Config.Pin = DISPLAY_I2C_SCL_GPIO_PIN;
        HAL_GPIO_Init(DISPLAY_I2C_SCL_GPIO_PORT, &GPIO_Config);
    } else if (hi2c->Instance == I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();

        GPIO_Config.Alternate = GPIO_AF9_I2C2;;
        GPIO_Config.Pin = INA3221_I2C_SDA_GPIO_PIN;
        HAL_GPIO_Init(INA3221_I2C_SDA_GPIO_PORT, &GPIO_Config);

        GPIO_Config.Alternate = GPIO_AF4_I2C2;
        GPIO_Config.Pin = INA3221_I2C_SCL_GPIO_PIN;
        HAL_GPIO_Init(INA3221_I2C_SCL_GPIO_PORT, &GPIO_Config);
    }
}