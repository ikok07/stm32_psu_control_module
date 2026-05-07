//
// Created by Kok on 4/29/26.
//

#ifndef STM32_F401_CBU6_GPIO_DEFS_H
#define STM32_F401_CBU6_GPIO_DEFS_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#define PWR_VALID_FLAG_GPIO_PORT                                    GPIOA
#define PWR_VALID_FLAG_GPIO_PIN                                     GPIO_PIN_0

#define PWR_CRITICAL_FLAG_GPIO_PORT                                 GPIOA
#define PWR_CRITICAL_FLAG_GPIO_PIN                                  GPIO_PIN_1

#define PWR_WARNING_FLAG_GPIO_PORT                                  GPIOA
#define PWR_WARNING_FLAG_GPIO_PIN                                   GPIO_PIN_2

#define PWR_TIMING_FLAG_GPIO_PORT                                   GPIOA
#define PWR_TIMING_FLAG_GPIO_PIN                                    GPIO_PIN_3

#define PWR_3V3_EN_FLAG_GPIO_PORT                                   GPIOA
#define PWR_3V3_EN_FLAG_GPIO_PIN                                    GPIO_PIN_4

#define PWR_5V_EN_FLAG_GPIO_PORT                                    GPIOA
#define PWR_5V_EN_FLAG_GPIO_PIN                                     GPIO_PIN_5

#define PWR_12V_EN_FLAG_GPIO_PORT                                   GPIOA
#define PWR_12V_EN_FLAG_GPIO_PIN                                    GPIO_PIN_6

#define PWR_SCREEN_SEL_GPIO_PORT                                    GPIOA
#define PWR_SCREEN_SEL_GPIO_PIN                                     GPIO_PIN_7

#define UART_TX_GPIO_PORT                                           GPIOA
#define UART_TX_GPIO_PIN                                            GPIO_PIN_9

#define ALARM_BUZZER_GPIO_PORT                                      GPIOA
#define ALARM_BUZZER_GPIO_PIN                                       GPIO_PIN_12

#define ERR_ACTIVE_FLAG_GPIO_PORT                                   GPIOA
#define ERR_ACTIVE_FLAG_GPIO_PIN                                    GPIO_PIN_15

#define INA3221_I2C_SDA_GPIO_PORT                                   GPIOB
#define INA3221_I2C_SDA_GPIO_PIN                                    GPIO_PIN_3

#define DISPLAY_I2C_SCL_GPIO_PORT                                   GPIOB
#define DISPLAY_I2C_SCL_GPIO_PIN                                    GPIO_PIN_6

#define DISPLAY_I2C_SDA_GPIO_PORT                                   GPIOB
#define DISPLAY_I2C_SDA_GPIO_PIN                                    GPIO_PIN_7

#define INA3221_I2C_SCL_GPIO_PORT                                   GPIOB
#define INA3221_I2C_SCL_GPIO_PIN                                    GPIO_PIN_10

#endif //STM32_F401_CBU6_GPIO_DEFS_H