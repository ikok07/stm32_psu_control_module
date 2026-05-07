//
// Created by Kok on 4/30/26.
//

#include "display.h"

#include <stdio.h>
#include <string.h>

#include "app_state.h"
#include "tasks_common.h"
#include "ssd1306.h"
#include "log.h"
#include "ssd1306_fonts.h"
#include "status_flags.h"

#define DISPLAY_I2C_SPEED_HZ                        100000
#define DISPLAY_WARNING_CYCLE_MS                    1000

I2C_HandleTypeDef SSD1306_I2C_PORT;

static void display_task(void *arg);

static void display_warning_timer_cb(TimerHandle_t xTimer);

static void display_show_page(uint32_t StatusFlags, INA3221_ReadResultTypeDef *Results);

static void display_draw_init();
static void display_draw_chan_disabled(const char *Title);
static void display_draw_error(const char *Label);
static void display_draw_data(const char *Title, float Power, float Current);

static uint8_t gWarningIcon[] = {
    0x00, 0x00, 0x06, 0x00, 0x09, 0x00, 0x09, 0x00, 0x16, 0x80, 0x16, 0x80, 0x26, 0x40, 0x60, 0x60,
    0x46, 0x20, 0x80, 0x10, 0x7f, 0xe0, 0x00, 0x00
};

static uint8_t gWarningShown;
static TimerHandle_t gWarningTimer;

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

    gWarningTimer = xTimerCreate(
        "Display warning timer",
        pdMS_TO_TICKS(DISPLAY_WARNING_CYCLE_MS),
        pdTRUE,
        gWarningTimer,
        display_warning_timer_cb
    );

    ssd1306_Init();

    display_draw_init();
    ssd1306_UpdateScreen();

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

    uint8_t init_load = 1;

    while (1) {
        if (init_load || xTaskNotifyWait(0x00, 0xFF, NULL, portMAX_DELAY)) {
            LOGGER_Log(LOGGER_LEVEL_INFO, "Display update triggered!");
            init_load = 0;

            if ((shval_err = SHVAL_PointerGetValue(&gAppState.SharedValues->MeasurementResults, results, NULL, 10)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared measurement results value! Error code: %d", shval_err);
                continue;
            }

            if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->StatusFlags, &status_flags, 10)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared status flags value! Error code: %d", shval_err);
                continue;
            }

            if (((status_flags >> STATUS_FLAG_PWR_CRITICAL) & 0x01) > 0) {
                xTimerStop(gWarningTimer, 0);
                display_draw_error("Critical power");
                xTaskNotify(gAppState.Tasks->AlarmBuzzerTask, STATUS_FLAG_ALARM_DURATION_MS, eNoAction);
                continue;
            }

            if (((status_flags >> STATUS_FLAG_PWR_WARNING) & 0x01) > 0) {
                display_draw_error("Power warning");
                gWarningShown = 1;
                xTimerStart(gWarningTimer, 0);
                xTaskNotify(gAppState.Tasks->AlarmBuzzerTask, STATUS_FLAG_ALARM_DURATION_MS, eNoAction);
                continue;
            }

            xTimerStop(gWarningTimer, 0);

            if (((status_flags >> STATUS_FLAG_PWR_VALID) & 0x01) == 0) {
                display_draw_error("PWR stabilizing");
                continue;
            }

            uint8_t curr_page = (status_flags >> STATUS_FLAG_PWR_DISPLAY_PAGE) & 0x03;
            float current_milliamps = (float)results[curr_page].MilliAmps;
            float power_milliwatts = ((float)results[curr_page].MilliVolts * current_milliamps) / 1000.0f;

            if (curr_page == PWR_DISPLAY_PAGE_3V3) {
                if (((status_flags >> STATUS_FLAG_3V3_EN) & 0x01) == 0) {
                    display_draw_chan_disabled("3V3 Rail");
                } else {
                    display_draw_data("3V3 Rail", power_milliwatts, current_milliamps);
                }
            } else if (curr_page == PWR_DISPLAY_PAGE_5V) {
                if (((status_flags >> STATUS_FLAG_5V_EN) & 0x01) == 0) {
                    display_draw_chan_disabled("5V Rail");
                } else {
                    display_draw_data("5V Rail", power_milliwatts, current_milliamps);
                }
            } else if (curr_page == PWR_DISPLAY_PAGE_12V) {
                if (((status_flags >> STATUS_FLAG_12V_EN) & 0x01) == 0) {
                    display_draw_chan_disabled("12V Rail");
                } else {
                    display_draw_data("12V Rail", power_milliwatts, current_milliamps);
                }
            }
        } else {
            LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to wait for shared measurements value! Error code: %d", shval_err);
        }
    }
}

void display_warning_timer_cb(TimerHandle_t xTimer) {
    if (gWarningShown) {
        SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;
        uint32_t status_flags;
        INA3221_ReadResultTypeDef results[3];

        if ((shval_err = SHVAL_PointerGetValue(&gAppState.SharedValues->MeasurementResults, results, NULL, 10)) != SHVAL_ERROR_OK) {
            LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared measurement results value! Error code: %d", shval_err);
            return;
        }

        if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->StatusFlags, &status_flags, 10)) != SHVAL_ERROR_OK) {
            LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared status flags value! Error code: %d", shval_err);
            return;
        }

        display_show_page(status_flags, results);
        gWarningShown = 0;
    } else {
        display_draw_error("Power warning");
        xTaskNotify(gAppState.Tasks->AlarmBuzzerTask, STATUS_FLAG_ALARM_DURATION_MS, eNoAction);
        gWarningShown = 1;
    }
}

void display_show_page(uint32_t StatusFlags, INA3221_ReadResultTypeDef *Results) {
    uint8_t curr_page = (StatusFlags >> STATUS_FLAG_PWR_DISPLAY_PAGE) & 0x03;
    float current_milliamps = (float)Results[curr_page].MilliAmps;
    float power_milliwatts = ((float)Results[curr_page].MilliVolts * current_milliamps) / 1000.0f;

    if (curr_page == PWR_DISPLAY_PAGE_3V3) {
        if (((StatusFlags >> STATUS_FLAG_3V3_EN) & 0x01) == 0) {
            display_draw_chan_disabled("3V3 Rail");
        } else {
            display_draw_data("3V3 Rail", power_milliwatts, current_milliamps);
        }
    } else if (curr_page == PWR_DISPLAY_PAGE_5V) {
        if (((StatusFlags >> STATUS_FLAG_5V_EN) & 0x01) == 0) {
            display_draw_chan_disabled("5V Rail");
        } else {
            display_draw_data("5V Rail", power_milliwatts, current_milliamps);
        }
    } else if (curr_page == PWR_DISPLAY_PAGE_12V) {
        if (((StatusFlags >> STATUS_FLAG_12V_EN) & 0x01) == 0) {
            display_draw_chan_disabled("12V Rail");
        } else {
            display_draw_data("12V Rail", power_milliwatts, current_milliamps);
        }
    }
}

void display_draw_init() {
    char buffer[] = "Initializing...";

    // Clear screen
    ssd1306_Fill(Black);

    // Clear screen
    ssd1306_Fill(Black);

    // Place in centre
    ssd1306_SetCursor((128 - (strlen(buffer) * 7)) / 2, 11);

    // Display text
    ssd1306_WriteString(buffer, Font_7x10, White);

    // Update display with new canvas data
    ssd1306_UpdateScreen();
}

void display_draw_chan_disabled(const char *Title) {
    uint8_t buffer[20];
    uint8_t x_cursor = 0, y_cursor = 0;

    // Clear screen
    ssd1306_Fill(Black);

    // Write title
    x_cursor = (128 - (strlen(Title) * 7)) / 2;
    y_cursor = 2;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(Title, Font_7x10, White);

    // Write disabled message
    snprintf(buffer, sizeof(buffer), "Channel disabled");
    x_cursor = (128 - (strlen(buffer) * 7)) / 2;
    y_cursor = 16;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(buffer, Font_7x10, White);

    // Update display with new canvas data
    ssd1306_UpdateScreen();
}

void display_draw_error(const char *Label) {
    uint8_t x_cursor = 0, y_cursor = 0;

    // Clear screen
    ssd1306_Fill(Black);

    x_cursor = (128 - sizeof(gWarningIcon)) / 2;
    ssd1306_DrawBitmap(x_cursor, y_cursor, gWarningIcon, 12, 12, White);

    x_cursor = (128 - strlen(Label) * 7) / 2;
    y_cursor = 16;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(Label, Font_7x10, White);

    // Update display with new canvas data
    ssd1306_UpdateScreen();
}

void display_draw_data(const char *Title, float Power, float Current) {
    char buffer[20];
    uint8_t x_cursor = 0, y_cursor = 0;

    // Clear screen
    ssd1306_Fill(Black);

    // Header
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString("mW", Font_7x10, White);

    x_cursor = (128 - (strlen(Title) * 7)) / 2;
    y_cursor = 2;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(Title, Font_7x10, White);

    x_cursor = 108;
    y_cursor = 0;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString("mA", Font_7x10, White);

    // Power
    snprintf(buffer, sizeof(buffer), "%#.4g", Power);
    x_cursor = 0;
    y_cursor = 14;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(buffer, Font_11x18, White);

    // Current
    snprintf(buffer, sizeof(buffer), "%#.4g", Current);
    x_cursor = 64;
    ssd1306_SetCursor(x_cursor, y_cursor);
    ssd1306_WriteString(buffer, Font_11x18, White);

    // Update display with new canvas data
    ssd1306_UpdateScreen();
}