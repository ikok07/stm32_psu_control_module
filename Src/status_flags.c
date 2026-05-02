//
// Created by Kok on 4/29/26.
//

#include "status_flags.h"

#include "app_state.h"
#include "gpio_defs.h"
#include "log.h"
#include "nvic_prio_defs.h"
#include "tasks_common.h"

#include "stm32f4xx_hal.h"

void status_flags_task(void *arg);

QueueHandle_t gStatusFlagsQueue;

void STFLAGS_Init() {
    gStatusFlagsQueue = xQueueCreate(5, sizeof(uint16_t));

    SHVAL_ConfigTypeDef shval_config = {
        .InitialValue = 0
    };
    gAppState.SharedValues->StatusFlags = SHVAL_Init(&shval_config);

    xTaskCreate(
        status_flags_task,
        "Status Flags Task",
        STATUS_FLAGS_TASK_STACK_DEPTH,
        NULL,
        STATUS_FLAGS_TASK_PRIORITY,
        &gAppState.Tasks->StatusFlagsTask
    );

    GPIO_InitTypeDef GPIO_Config = {
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Mode = GPIO_MODE_IT_RISING_FALLING
    };

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

    GPIO_Config.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Config.Pin = PWR_3V3_EN_FLAG_GPIO_PIN;
    GPIO_Config.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(PWR_3V3_EN_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI4_IRQn, PWR_3V3_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    GPIO_Config.Pin = PWR_5V_EN_FLAG_GPIO_PIN;
    HAL_GPIO_Init(PWR_5V_EN_FLAG_GPIO_PORT, &GPIO_Config);

    GPIO_Config.Pin = PWR_12V_EN_FLAG_GPIO_PIN;
    HAL_GPIO_Init(PWR_12V_EN_FLAG_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, PWR_5V_12V_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_Config.Pin = PWR_SCREEN_SEL_GPIO_PIN;
    GPIO_Config.Mode = GPIO_MODE_IT_RISING;
    HAL_GPIO_Init(PWR_SCREEN_SEL_GPIO_PORT, &GPIO_Config);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, PWR_5V_12V_FLAG_GPIO_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_Config.Pin = ERR_ACTIVE_FLAG_GPIO_PIN;
    HAL_GPIO_Init(ERR_ACTIVE_FLAG_GPIO_PORT, &GPIO_Config);
}

void status_flags_task(void *arg) {
    LOGGER_Log(LOGGER_LEVEL_INFO, "Status flags tasks started!");

    SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;
    uint32_t curr_flags = 0;
    uint16_t gpio_pin;
    uint32_t last_tick = 0;

    while (1) {
        if (xQueueReceive(gStatusFlagsQueue, &gpio_pin, portMAX_DELAY)) {
            if (HAL_GetTick() - last_tick < 150) continue; // Ignore within 50ms
            last_tick = HAL_GetTick();

            if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->StatusFlags, &curr_flags, 1000)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared status flags! Error code: %d", shval_err);
                continue;
            }

            // GPIOA is common for all GPIOs
            uint8_t curr_gpio_state = HAL_GPIO_ReadPin(GPIOA, gpio_pin);

            switch (gpio_pin) {
                case PWR_VALID_FLAG_GPIO_PIN:
                    if (curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_PWR_VALID);
                    else curr_flags &=~ (1 << STATUS_FLAG_PWR_VALID);
                    break;
                case PWR_CRITICAL_FLAG_GPIO_PIN:
                    if (!curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_PWR_CRITICAL);
                    else curr_flags &=~ (1 << STATUS_FLAG_PWR_CRITICAL);
                    break;
                case PWR_WARNING_FLAG_GPIO_PIN:
                    if (!curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_PWR_WARNING);
                    else curr_flags &=~ (1 << STATUS_FLAG_PWR_WARNING);
                    break;
                case PWR_TIMING_FLAG_GPIO_PIN:
                    if (!curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_PWR_TIMING);
                    else curr_flags &=~ (1 << STATUS_FLAG_PWR_TIMING);
                    break;
                case PWR_3V3_EN_FLAG_GPIO_PIN:
                    if (curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_3V3_EN);
                    else curr_flags &=~ (1 << STATUS_FLAG_3V3_EN);
                    break;
                case PWR_5V_EN_FLAG_GPIO_PIN:
                    if (curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_5V_EN);
                    else curr_flags &=~ (1 << STATUS_FLAG_5V_EN);
                    break;
                case PWR_12V_EN_FLAG_GPIO_PIN:
                    if (curr_gpio_state) curr_flags |= (1 << STATUS_FLAG_12V_EN);
                    else curr_flags &=~ (1 << STATUS_FLAG_12V_EN);
                    break;
                case PWR_SCREEN_SEL_GPIO_PIN:
                    uint8_t curr_mode = (curr_flags >> STATUS_FLAG_PWR_DISPLAY_PAGE) & 0x03;
                    uint8_t next_mode = curr_mode >= PWR_DISPLAY_PAGE_12V ? PWR_DISPLAY_PAGE_3V3 : curr_mode + 1;

                    // Clear bits
                    curr_flags &=~ (0x03 << STATUS_FLAG_PWR_DISPLAY_PAGE);

                    // Set new bits
                    curr_flags |= (next_mode << STATUS_FLAG_PWR_DISPLAY_PAGE);
                    break;
            }

            if ((shval_err = SHVAL_SetValue(&gAppState.SharedValues->StatusFlags, curr_flags, 1000)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to set shared status flags! Error code: %d", shval_err);
            };
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(gStatusFlagsQueue, &GPIO_Pin, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}