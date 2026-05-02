//
// Created by Kok on 4/9/26.
//

#include "FreeRTOS.h"
#include "task.h"
#include "logging.h"

#include <string.h>

#include "app_state.h"
#include "log.h"
#include "stm32f4xx_hal.h"

#define LOGGING_ERR_LED_GPIOEN                  __HAL_RCC_GPIOA_CLK_ENABLE()
#define LOGGING_ERR_LED_PORT                    GPIOA
#define LOGGING_ERR_LED_PIN                     15

#define LOGGING_UART_TIMEOUT                    1000

static void init_basic_logger();
static void fatal_basic_err_cb();

static uint8_t init_logger();
static uint8_t log_cb(LOGGER_EventTypeDef *event);
static uint8_t fatal_err_cb(LOGGER_EventTypeDef *event);

void LOGGING_Init() {
    LOGGER_CallbacksTypeDef callbacks = {
        .on_init_basic = init_basic_logger,
        .on_fatal_basic = fatal_basic_err_cb,
        .on_fatal_err = fatal_err_cb,
        .on_init = init_logger,
        .on_log = log_cb,
    };

    LOGGER_RegisterCB(&callbacks);
    if (LOGGER_Init() != LOGGER_ERROR_OK) {
        // Basic logger will be ready no matter what happens
        LOGGER_Enable();
        LOGGER_LogBasic(1);
    }
    LOGGER_Enable();
}

void init_basic_logger() {
    LOGGING_ERR_LED_GPIOEN;

    GPIO_InitTypeDef GPIO_Config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pin = LOGGING_ERR_LED_PIN,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_LOW
    };
    HAL_GPIO_Init(LOGGING_ERR_LED_PORT, &GPIO_Config);
}

void fatal_basic_err_cb() {
    HAL_GPIO_WritePin(LOGGING_ERR_LED_PORT, LOGGING_ERR_LED_PIN, GPIO_PIN_SET);
    vTaskEndScheduler();
    __disable_irq();
    while (1);
}

uint8_t init_logger() {
    HAL_StatusTypeDef hal_err = HAL_OK;

    gAppState.huart = (UART_HandleTypeDef){
        .Instance = USART1,
        .Init = {
            .Mode = UART_MODE_TX,
            .BaudRate = 9600,
            .HwFlowCtl = UART_HWCONTROL_NONE,
            .OverSampling = UART_OVERSAMPLING_16,
            .Parity = UART_PARITY_NONE,
            .StopBits = UART_STOPBITS_1,
            .WordLength = UART_WORDLENGTH_8B
        }
    };

    if ((hal_err = HAL_UART_Init(&gAppState.huart)) != HAL_OK) return hal_err;
    return hal_err;
}

uint8_t log_cb(LOGGER_EventTypeDef *event) {
    HAL_StatusTypeDef hal_err = HAL_OK;
    if ((hal_err = HAL_UART_Transmit(&gAppState.huart, (uint8_t*)event->msg, strlen(event->msg) + 1, LOGGING_UART_TIMEOUT)) != HAL_OK) {
        return hal_err;
    };
    HAL_GPIO_WritePin(LOGGING_ERR_LED_PORT, LOGGING_ERR_LED_PIN, GPIO_PIN_SET);
    return hal_err;
}

uint8_t fatal_err_cb(LOGGER_EventTypeDef *event) {
    HAL_StatusTypeDef hal_err = HAL_OK;
    if ((hal_err = HAL_UART_Transmit(&gAppState.huart, (uint8_t*)event->msg, strlen(event->msg) + 1, LOGGING_UART_TIMEOUT)) != HAL_OK) {
        return hal_err;
    };
    HAL_GPIO_WritePin(LOGGING_ERR_LED_PORT, LOGGING_ERR_LED_PIN, GPIO_PIN_SET);
    vTaskEndScheduler();
    __disable_irq();
    while (1);
}
