//
// Created by Kok on 4/29/26.
//


#ifndef STM32_F401_CBU6_STATUS_FLAGS_H
#define STM32_F401_CBU6_STATUS_FLAGS_H

#include <stdint.h>

#define STATUS_FLAG_PWR_VALID                           0
#define STATUS_FLAG_PWR_CRITICAL                        1
#define STATUS_FLAG_PWR_WARNING                         2
#define STATUS_FLAG_PWR_TIMING                          3
#define STATUS_FLAG_3V3_EN                              4
#define STATUS_FLAG_5V_EN                               5
#define STATUS_FLAG_12V_EN                              6
#define STATUS_FLAG_PWR_DISPLAY_PAGE                    7           // 2 bits for 3 pages
#define STATUS_FLAG_ERR_ACTIVE                          9

#define PWR_DISPLAY_PAGE_3V3                            0
#define PWR_DISPLAY_PAGE_5V                             1
#define PWR_DISPLAY_PAGE_12V                            2

#define STATUS_FLAG_ALARM_DURATION_MS                   2000

typedef struct {
    uint16_t Pin;
    uint8_t State;
} STFLAGS_EventTypeDef;

void STFLAGS_Init();

#endif //STM32_F401_CBU6_STATUS_FLAGS_H