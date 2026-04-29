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
#define PWR_3V3_FLAG_GPIO_PIN                                       GPIO_PIN_4

#define PWR_5V_EN_FLAG_GPIO_PORT                                    GPIOA
#define PWR_5V_FLAG_GPIO_PIN                                        GPIO_PIN_5

#define PWR_12V_EN_FLAG_GPIO_PORT                                   GPIOA
#define PWR_12V_FLAG_GPIO_PIN                                       GPIO_PIN_6

#define ERR_ACTIVE_FLAG_GPIO_PORT                                   GPIOA
#define ERR_ACTIVE_FLAG_GPIO_PIN                                    GPIO_PIN_15

#endif //STM32_F401_CBU6_GPIO_DEFS_H