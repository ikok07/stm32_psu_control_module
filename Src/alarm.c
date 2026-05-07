//
// Created by Kok on 5/7/26.
//

#include "alarm.h"

#include "app_state.h"
#include "gpio_defs.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tasks_common.h"

static void alarm_buzzer_task(void *arg);

void ALARM_Init() {
    GPIO_InitTypeDef GPIO_Config = {
        .Pin = ALARM_BUZZER_GPIO_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(ALARM_BUZZER_GPIO_PORT, &GPIO_Config);

    xTaskCreate(
        alarm_buzzer_task,
        "Alram buzzer task",
        ALARM_BUZZER_TASK_STACK_DEPTH,
        NULL,
        ALARM_BUZZER_TASK_PRIORITY,
        &gAppState.Tasks->AlarmBuzzerTask
    );
}

void alarm_buzzer_task(void *arg) {
    while (1) {
        uint32_t duration_ms;
        if (xTaskNotifyWait(0, 0xFF, &duration_ms, (duration_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(duration_ms))) {
            HAL_GPIO_WritePin(ALARM_BUZZER_GPIO_PORT, ALARM_BUZZER_GPIO_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(ALARM_BUZZER_GPIO_PORT, ALARM_BUZZER_GPIO_PIN, GPIO_PIN_RESET);
            duration_ms = 0;
        }
    }
}