//
// Created by Kok on 4/29/26.
//

#include "status_flags.h"

#include <string.h>

#include "app_state.h"
#include "bit_defs.h"
#include "gpio_defs.h"
#include "log.h"
#include "nvic_prio_defs.h"
#include "tasks_common.h"

#include "stm32f4xx_hal.h"

void status_flags_task(void *arg);

static uint32_t handle_status_change(uint16_t GpioPin, uint8_t State, uint32_t CurrFlags);

QueueHandle_t gStatusFlagsQueue;

void STFLAGS_Init() {
    gStatusFlagsQueue = xQueueCreate(16, sizeof(STFLAGS_EventTypeDef));

    SHVAL_ConfigTypeDef shval_config = {
        .InitialValue = 0
    };
    gAppState.SharedValues->StatusFlags = SHVAL_Init(&shval_config);

    GPIO_InitTypeDef GPIO_Config = {
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Mode = GPIO_MODE_IT_RISING_FALLING
    };

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_Config.Pin = PWR_VALID_FLAG_GPIO_PIN;
    GPIO_Config.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(PWR_VALID_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI0_IRQn, PWR_VALID_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    GPIO_Config.Pin = PWR_CRITICAL_FLAG_GPIO_PIN;
    GPIO_Config.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PWR_CRITICAL_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI1_IRQn, PWR_CRITICAL_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    GPIO_Config.Pin = PWR_WARNING_FLAG_GPIO_PIN;
    GPIO_Config.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PWR_WARNING_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI2_IRQn, PWR_WARNING_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    GPIO_Config.Pin = PWR_TIMING_FLAG_GPIO_PIN;
    GPIO_Config.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PWR_TIMING_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI3_IRQn, PWR_TIMING_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    GPIO_Config.Pin = PWR_3V3_EN_FLAG_GPIO_PIN;
    GPIO_Config.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(PWR_3V3_EN_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI4_IRQn, PWR_3V3_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    GPIO_Config.Pin = PWR_5V_EN_FLAG_GPIO_PIN;
    HAL_GPIO_Init(PWR_5V_EN_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, PWR_5V_12V_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_Config.Pin = PWR_12V_EN_FLAG_GPIO_PIN;
    HAL_GPIO_Init(PWR_12V_EN_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, PWR_5V_12V_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_Config.Mode = GPIO_MODE_IT_RISING;
    GPIO_Config.Pin = PWR_SCREEN_SEL_GPIO_PIN;
    HAL_GPIO_Init(PWR_SCREEN_SEL_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, PWR_5V_12V_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_Config.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Config.Pin = ERR_ACTIVE_FLAG_GPIO_PIN;
    HAL_GPIO_Init(ERR_ACTIVE_FLAG_GPIO_PORT, &GPIO_Config);

    // Handle initial gpio states
    uint32_t curr_flags = handle_status_change(PWR_VALID_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_VALID_FLAG_GPIO_PIN), 0);
    curr_flags = handle_status_change(PWR_CRITICAL_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_CRITICAL_FLAG_GPIO_PIN), curr_flags);
    curr_flags = handle_status_change(PWR_WARNING_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_WARNING_FLAG_GPIO_PIN), curr_flags);
    curr_flags = handle_status_change(PWR_TIMING_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_TIMING_FLAG_GPIO_PIN), curr_flags);
    curr_flags = handle_status_change(PWR_3V3_EN_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_3V3_EN_FLAG_GPIO_PIN), curr_flags);
    curr_flags = handle_status_change(PWR_5V_EN_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_5V_EN_FLAG_GPIO_PIN), curr_flags);
    handle_status_change(PWR_12V_EN_FLAG_GPIO_PIN, HAL_GPIO_ReadPin(GPIOA, PWR_12V_EN_FLAG_GPIO_PIN), curr_flags);

    xTaskCreate(
    status_flags_task,
    "Status Flags Task",
    STATUS_FLAGS_TASK_STACK_DEPTH,
    NULL,
    STATUS_FLAGS_TASK_PRIORITY,
    &gAppState.Tasks->StatusFlagsTask
);
}

void status_flags_task(void *arg) {
    SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;
    uint32_t curr_flags = 0;
    STFLAGS_EventTypeDef event;
    // Track for each GPIO
    TickType_t last_ticks[16] = {0};
    uint8_t last_states[16] = {0};
    memset(last_states, 2, sizeof(last_states));

    while (1) {
        if (xQueueReceive(gStatusFlagsQueue, &event, portMAX_DELAY)) {
            uint8_t pin_idx = __builtin_ctz(event.Pin);
            TickType_t now = xTaskGetTickCount();

            // 25ms debounce
            if (now - last_ticks[pin_idx] < pdMS_TO_TICKS(25) || event.State == last_states[pin_idx]) continue;
            last_ticks[pin_idx] = now;
            last_states[pin_idx] = event.State;

            if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->StatusFlags, &curr_flags, 1000)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared status flags! Error code: %d", shval_err);
                continue;
            }

            // GPIOA is common for all GPIOs
            curr_flags = handle_status_change(event.Pin, event.State, curr_flags);

            xTaskNotifyGive(gAppState.Tasks->DisplayTask);
        }
    }
}

uint32_t handle_status_change(uint16_t GpioPin, uint8_t State, uint32_t CurrFlags) {
    SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;

    switch (GpioPin) {
        case PWR_VALID_FLAG_GPIO_PIN:
            if (State) CurrFlags |= (1 << STATUS_FLAG_PWR_VALID);
            else CurrFlags &=~ (1 << STATUS_FLAG_PWR_VALID);
            break;
        case PWR_CRITICAL_FLAG_GPIO_PIN:
            if (!State) CurrFlags |= (1 << STATUS_FLAG_PWR_CRITICAL);
            else CurrFlags &=~ (1 << STATUS_FLAG_PWR_CRITICAL);
            break;
        case PWR_WARNING_FLAG_GPIO_PIN:
            if (!State) CurrFlags |= (1 << STATUS_FLAG_PWR_WARNING);
            else CurrFlags &=~ (1 << STATUS_FLAG_PWR_WARNING);
            break;
        case PWR_TIMING_FLAG_GPIO_PIN:
            if (!State) CurrFlags |= (1 << STATUS_FLAG_PWR_TIMING);
            else CurrFlags &=~ (1 << STATUS_FLAG_PWR_TIMING);
            break;
        case PWR_3V3_EN_FLAG_GPIO_PIN:
            if (State) CurrFlags |= (1 << STATUS_FLAG_3V3_EN);
            else CurrFlags &=~ (1 << STATUS_FLAG_3V3_EN);
            break;
        case PWR_5V_EN_FLAG_GPIO_PIN:
            if (State) CurrFlags |= (1 << STATUS_FLAG_5V_EN);
            else CurrFlags &=~ (1 << STATUS_FLAG_5V_EN);
            break;
        case PWR_12V_EN_FLAG_GPIO_PIN:
            if (State) CurrFlags |= (1 << STATUS_FLAG_12V_EN);
            else CurrFlags &=~ (1 << STATUS_FLAG_12V_EN);
            break;
        case PWR_SCREEN_SEL_GPIO_PIN:
            uint8_t curr_mode = (CurrFlags >> STATUS_FLAG_PWR_DISPLAY_PAGE) & 0x03;
            uint8_t next_mode = curr_mode >= PWR_DISPLAY_PAGE_12V ? PWR_DISPLAY_PAGE_3V3 : curr_mode + 1;

            // Clear bits
            CurrFlags &=~ (0x03 << STATUS_FLAG_PWR_DISPLAY_PAGE);

            // Set new bits
            CurrFlags |= (next_mode << STATUS_FLAG_PWR_DISPLAY_PAGE);
            break;
    }

    if ((shval_err = SHVAL_SetValue(&gAppState.SharedValues->StatusFlags, CurrFlags, 1000)) != SHVAL_ERROR_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to set shared status flags! Error code: %d", shval_err);
    };

    return CurrFlags;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    STFLAGS_EventTypeDef event = {
        .Pin = GPIO_Pin,
        .State = HAL_GPIO_ReadPin(GPIOA, GPIO_Pin)
    };

    xQueueSendFromISR(gStatusFlagsQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}