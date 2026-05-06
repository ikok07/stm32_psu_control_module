#include "stm32f4xx_hal.h"

#include "app_state.h"
#include "bit_defs.h"
#include "display.h"
#include "log.h"
#include "logging.h"
#include "pwr_monitor.h"
#include "status_flags.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName ) {
    while (1);
}

int main(void) {
    // Initialize application state
    APP_Init();

    // Initialize the HAL driver
    HAL_Init();

    // Initialize logger
    LOGGING_Init();
    LOGGER_Log(LOGGER_LEVEL_INFO, "Logger initialized!");

    // Initialize status flags
    STFLAGS_Init();
    LOGGER_Log(LOGGER_LEVEL_INFO, "Status flags initialized!");

    // Initialize INA3221 power monitor
    PWRMONITOR_Init();
    LOGGER_Log(LOGGER_LEVEL_INFO, "INA3221 power monitor initialized!");

    // Initialize display
    DISPLAY_Init();
    LOGGER_Log(LOGGER_LEVEL_INFO, "Display initialized!");

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();
}
