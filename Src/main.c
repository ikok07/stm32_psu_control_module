#include "stm32f4xx_hal.h"

#include "app_state.h"
#include "log.h"
#include "logging.h"
#include "status_flags.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName ) {
    while (1);
}

int main(void) {
    if (HAL_Init() != HAL_OK) {
        while (1);
    }

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();

    // Initialize logger
    LOGGING_Init();

    LOGGER_Log(LOGGER_LEVEL_INFO, "Logger initialized!");

    // Initialize application state
    APP_Init();

    LOGGER_Log(LOGGER_LEVEL_INFO, "Application state!");

    // Initialize status flags
    STFLAGS_Init();

    LOGGER_Log(LOGGER_LEVEL_INFO, "Status flags initialized!");
}
