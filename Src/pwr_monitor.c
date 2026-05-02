//
// Created by Kok on 5/1/26.
//

#include "pwr_monitor.h"

#include "FreeRTOS.h"
#include "app_state.h"
#include "bit_defs.h"
#include "ina3221.h"
#include "log.h"
#include "tasks_common.h"

#define PWR_MONITOR_DEVICE_ADDRESS                              0x40

// 140us bus + 140us shunt; 3 channels; 64 samples → 53.76ms + 5ms margin = 58.76ms → 59ms
#define PWR_MONITOR_POLL_PERIOD_MS                              59

static TimerHandle_t gPowerMonitorReadTimer;
static INA3221_ReadResultTypeDef gResults[3];

void power_monitor_task(void *arg);

void power_monitor_read_timer_cb(TimerHandle_t xTimer);

uint8_t i2c_send(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t Data);
uint8_t i2c_read(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t *RegContents);
void log_error(INA3221_ErrorTypeDef DeviceError, uint8_t SpecificErrorCode);

void PWRMONITOR_Init() {
    HAL_StatusTypeDef hal_err = HAL_OK;
    INA3221_ErrorTypeDef ina3221_err = INA3221_ERROR_OK;

    gAppState.hi2c2.Instance = I2C2;
    gAppState.hi2c2.Init = (I2C_InitTypeDef){
        .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
        .ClockSpeed = 100000,
    };

    if ((hal_err = HAL_I2C_Init(&gAppState.hi2c2)) != HAL_OK) {
        LOGGER_LogF(LOGGER_LEVEL_FATAL, "Failed to initialize I2C2 peripheral! Error code: %d", hal_err);
        return;
    }

    gAppState.hina3221.Callbacks = (INA3221_CallbacksTypeDef){
        .I2CSend = i2c_send,
        .I2CRead = i2c_read,
        .LogError = log_error
    };
    gAppState.hina3221.Config = (INA3211_ConfigTypeDef){
        .Mode = INA3221_MODE_SHUNT_BUS_CONTINUOUS,
        .AveragingSamples = INA3221_AVG_SAMPLES_64,
        .BusConvTime = INA3221_CONV_140US,
        .ShuntConvTime = INA3221_CONV_140US,
        .CriticalAlertLatchEnabled = 0,
        .WarningAlertLatchEnabled = 0,
        .DeviceAddress = PWR_MONITOR_DEVICE_ADDRESS,
        .EnabledChannels = INA3221_CH1 | INA3221_CH2 | INA3221_CH3,
        .ShuntResistanceCh1MilliOhms = 10,
        .ShuntResistanceCh2MilliOhms = 8,
        .ShuntResistanceCh3MilliOhms = 5,
        .PowerValidUpperValueMilliVolts = 3300,
        .PowerValidLowerValueMilliVolts = 2500,
        .CriticalAlertValueCh1MicroVolts = 140000, // Max 14A (14A * 10mOhm)
        .CriticalAlertValueCh2MicroVolts = 144000, // Max 18A (18A * 8mOhm)
        .CriticalAlertValueCh3MicroVolts = 135000, // Max 27A (27A * 5mOhm)
        .WarningAlertValueCh1MicroVolts = 100000, // Warn at 10A
        .WarningAlertValueCh2MicroVolts = 112000, // Warn at 14A
        .WarningAlertValueCh3MicroVolts = 115000, // Warn at 23A
    };
    // if ((ina3221_err = INA3221_Init(&gAppState.hina3221)) != INA3221_ERROR_OK) {
    //     LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to initialize INA3221! Error code: %d", ina3221_err);
    //     return;
    // }

    SHVAL_PointerConfigTypeDef SHVAL_Config = {
        .InitialValue = gResults,
        .ValueLen = sizeof(gResults),
        .SubscribersEventBits = PWRMONITOR_DISPLAY_EVT_BIT,
    };

    gAppState.SharedValues->MeasurementResults = SHVAL_PointerInit(&SHVAL_Config);

    gPowerMonitorReadTimer = xTimerCreate(
      "Power Monitor Read timer",
      pdMS_TO_TICKS(PWR_MONITOR_POLL_PERIOD_MS),
      pdTRUE,
      NULL,
      power_monitor_read_timer_cb
    );
    xTimerStart(gPowerMonitorReadTimer, pdMS_TO_TICKS(100));

    xTaskCreate(
        power_monitor_task,
        "Power monitor task",
        POWER_MONITOR_TASK_STACK_DEPTH,
        NULL,
        POWER_MONITOR_TASK_PRIORITY,
        &gAppState.Tasks->PowerMonitorTask
    );
}

void power_monitor_read_timer_cb(TimerHandle_t xTimer) {
    if (gAppState.Tasks->PowerMonitorTask == NULL) return;
    xTaskNotifyGive(gAppState.Tasks->PowerMonitorTask);
}

uint8_t i2c_send(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t Data) {
    HAL_StatusTypeDef hal_err = HAL_OK;

    uint8_t buffer[3] = {RegisterAddress};
    buffer[1] = (Data >> 8) & 0xFF;
    buffer[2] = Data & 0xFF;

    if ((hal_err = HAL_I2C_Master_Transmit(&gAppState.hi2c2, DeviceAddress, buffer, sizeof(buffer), pdMS_TO_TICKS(100))) != HAL_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to write I2C data to INA3221! Error code: %d", hal_err);
        return 1;
    }

    return hal_err;
}

uint8_t i2c_read(uint8_t DeviceAddress, uint8_t RegisterAddress, uint16_t *RegContents) {
    HAL_StatusTypeDef hal_err = HAL_OK;

    uint8_t buffer[2];

    if ((hal_err = HAL_I2C_Master_Transmit(&gAppState.hi2c2, DeviceAddress, &RegisterAddress, 1, pdMS_TO_TICKS(100))) != HAL_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to request I2C data from INA3221! Error code: %d", hal_err);
        return hal_err;
    }

    if ((hal_err = HAL_I2C_Master_Receive(&gAppState.hi2c2, DeviceAddress, buffer, sizeof(buffer), pdMS_TO_TICKS(100))) != HAL_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to receive I2C data from INA3221! Error code: %d", hal_err);
        return hal_err;
    }

    *RegContents = (buffer[0] << 8) | buffer[1];
    return hal_err;
}

void log_error(INA3221_ErrorTypeDef DeviceError, uint8_t SpecificErrorCode) {
    LOGGER_LogF(LOGGER_LEVEL_ERROR, "Power monitor error occurred! INA3221 error: %d; Specific error: %d", DeviceError, SpecificErrorCode);
}

void power_monitor_task(void *arg) {
    INA3221_ErrorTypeDef ina_err = INA3221_ERROR_OK;
    SHVAL_ErrorTypeDef shval_err = SHVAL_ERROR_OK;
    INA3221_ReadResultTypeDef results[3];
    results[0] = (INA3221_ReadResultTypeDef){.MilliAmps = 100, .MilliVolts = 100};
    results[1] = (INA3221_ReadResultTypeDef){.MilliAmps = 200, .MilliVolts = 200};
    results[2] = (INA3221_ReadResultTypeDef){.MilliAmps = 300, .MilliVolts = 300};

    while (1) {
        if (xTaskNotifyWait(0x00, 0xFF, NULL, portMAX_DELAY)) {
            // if ((ina_err = INA3221_ReadChan(&gAppState.hina3221, INA3221_CH1, &results[0])) != INA3221_ERROR_OK) {
            //     LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to read INA3221 channel 1! Error code: %d", ina_err);
            //     continue;
            // };
            //
            // if ((ina_err = INA3221_ReadChan(&gAppState.hina3221, INA3221_CH2, &results[1])) != INA3221_ERROR_OK) {
            //     LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to read INA3221 channel 2! Error code: %d", ina_err);
            //     continue;
            // };
            //
            // if ((ina_err = INA3221_ReadChan(&gAppState.hina3221, INA3221_CH3, &results[2])) != INA3221_ERROR_OK) {
            //     LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to read INA3221 channel 3! Error code: %d", ina_err);
            //     continue;
            // };
            vTaskDelay(1000);
            if ((shval_err = SHVAL_PointerSetValue(&gAppState.SharedValues->MeasurementResults, results, 10)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to set shared measurement results! Error code: %d", shval_err);
                continue;
            }
        }
    }
}
