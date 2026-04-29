//
// Created by Kok on 4/29/26.
//

#include "app_state.h"

APP_TasksTypeDef tasks;
APP_SharedValuesTypeDef shared_values;

APP_StateTypeDef gAppState;

void APP_Init() {
    gAppState = (APP_StateTypeDef){
        .Tasks = &tasks,
        .SharedValues = &shared_values
    };
}