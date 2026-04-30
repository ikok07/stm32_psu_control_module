//
// Created by Kok on 4/29/26.
//

#ifndef STM32_F401_CBU6_APP_STATE_H
#define STM32_F401_CBU6_APP_STATE_H

#include "ina3221.h"
#include "shared_values.h"
#include "stm32f4xx_hal.h"

typedef struct {
    TaskHandle_t StatusFlagsTask;
} APP_TasksTypeDef;

typedef struct {
    SHVAL_HandleTypeDef StatusFlags;
} APP_SharedValuesTypeDef;

typedef struct {
    INA3221_HandleTypeDef hina3221;
    TIM_HandleTypeDef htim9;                        // User for HAL Ticks!!!
    UART_HandleTypeDef huart;
    APP_TasksTypeDef *Tasks;
    APP_SharedValuesTypeDef *SharedValues;
} APP_StateTypeDef;

extern APP_StateTypeDef gAppState;

void APP_Init();

#endif //STM32_F401_CBU6_APP_STATE_H