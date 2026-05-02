//
// Created by Kok on 4/30/26.
//

#include "display.h"

#include <stdio.h>
#include <string.h>

#include "app_state.h"
#include "bit_defs.h"
#include "tasks_common.h"
#include "ssd1306.h"
#include "log.h"
#include "pwr_monitor.h"
#include "ssd1306_fonts.h"
#include "status_flags.h"

#define DISPLAY_I2C_SPEED_HZ                        100000

I2C_HandleTypeDef SSD1306_I2C_PORT;

static void display_task(void *arg);

static void display_data(const char *title, float power, float current);
static void display_init();

void DISPLAY_Init() {
    HAL_StatusTypeDef hal_err = HAL_OK;
    SSD1306_I2C_PORT.Instance = I2C1;
    SSD1306_I2C_PORT.Init = (I2C_InitTypeDef){
        .ClockSpeed = DISPLAY_I2C_SPEED_HZ,
        .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
        .DualAddressMode = I2C_DUALADDRESS_DISABLE,
        .GeneralCallMode = I2C_GENERALCALL_DISABLE,
        .NoStretchMode = I2C_NOSTRETCH_DISABLE,
    };

    if ((hal_err = HAL_I2C_Init(&SSD1306_I2C_PORT)) != HAL_OK) {
        LOGGER_LogF(LOGGER_LEVEL_FATAL, "Failed to initialize I2C! peripheral! Error code: %d", hal_err);
        return;
    };

    if ((hal_err = HAL_I2C_IsDeviceReady(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 3, pdMS_TO_TICKS(1000))) != HAL_OK) {
        LOGGER_LogF(LOGGER_LEVEL_FATAL, "Failed to establish I2C connection with display! Error code: %d", hal_err);
        return;
    }

    ssd1306_Init();

    xTaskCreate(
        display_task,
        "Display task",
        DISPLAY_TASK_STACK_DEPTH,
        NULL,
        DISPLAY_TASK_PRIORITY,
        &gAppState.Tasks->DisplayTask
    );
}

void display_task(void *arg) {
    SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;
    INA3221_ReadResultTypeDef results[3];
    uint32_t status_flags;
    uint8_t initial_load = 1;
    while (1) {
        if (initial_load) {
            display_init();
            ssd1306_UpdateScreen();
            initial_load = 0;
        }
        if ((shval_err = SHVAL_PointerWaitForValue(&gAppState.SharedValues->MeasurementResults, PWRMONITOR_DISPLAY_EVT_BIT, results, NULL, portMAX_DELAY)) == SHVAL_ERROR_OK) {

            if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->StatusFlags, &status_flags, pdMS_TO_TICKS(10))) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared status flags variable! Error code: %d", shval_err);
            }

            uint8_t curr_page = (status_flags >> STATUS_FLAG_PWR_DISPLAY_PAGE) & 0x03;
            float current_milliamps = (float)results[curr_page].MilliAmps;
            float power_milliwatts = ((float)results[curr_page].MilliVolts * current_milliamps) / 1000.0f;

            ssd1306_Fill(Black);
            if (curr_page == PWR_DISPLAY_PAGE_3V3) {
                display_data("3V3 Rail", power_milliwatts, current_milliamps);
            } else if (curr_page == PWR_DISPLAY_PAGE_5V) {
                display_data("5V Rail", power_milliwatts, current_milliamps);
            } else if (curr_page == PWR_DISPLAY_PAGE_12V) {
                display_data("12V Rail", power_milliwatts, current_milliamps);
            }

            // Update display with new canvas data
            ssd1306_UpdateScreen();
        }
    }
}

void display_data(const char *title, float power, float current) {
    char buffer[20];
    uint8_t x_cursor = 0, y_cursor = 0;

    // Header
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString("mW", Font_7x10, White);

    x_cursor = (128 - (strlen(title) * 7)) / 2;
    y_cursor = 2;
    snprintf(buffer, sizeof(buffer), title);
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(buffer, Font_7x10, White);

    x_cursor = 108;
    y_cursor = 0;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString("mA", Font_7x10, White);

    // Power
    snprintf(buffer, sizeof(buffer), "%#.4g", power);
    x_cursor = 0;
    y_cursor = 14;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(buffer, Font_11x18, White);

    // Current
    snprintf(buffer, sizeof(buffer), "%#.4g", current);
    x_cursor = 64;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(buffer, Font_11x18, White);
}

void display_init() {
    char buffer[] = "Initializing...";

    // Clear screen
    ssd1306_Fill(Black);

    // Place in centre
    ssd1306_SetCursor((128 - (strlen(buffer) * 7)) / 2, 11);

    // Display text
    ssd1306_WriteString(buffer, Font_7x10, White);
}