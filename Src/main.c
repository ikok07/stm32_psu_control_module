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

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();
}
