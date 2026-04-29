//
// Created by Kok on 4/29/26.
//


#ifndef STM32_F401_CBU6_STATUS_FLAGS_H
#define STM32_F401_CBU6_STATUS_FLAGS_H

#include "bit_defs.h"

#define STATUS_FLAGS_EVT_BIT                            BIT0

#define STATUS_FLAG_PWR_VALID                           0
#define STATUS_FLAG_PWR_CRITICAL                        1
#define STATUS_FLAG_PWR_WARNING                         2
#define STATUS_FLAG_PWR_TIMING                          3
#define STATUS_FLAG_3V3_EN_                             4
#define STATUS_FLAG_5V_EN                               5
#define STATUS_FLAG_12V_EN                              6
#define STATUS_FLAG_ERR_ACTIVE                          7

void STFLAGS_Init();

#endif //STM32_F401_CBU6_STATUS_FLAGS_H