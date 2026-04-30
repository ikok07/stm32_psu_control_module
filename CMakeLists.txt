cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32_f401_cbu6)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

file(GLOB APP_SOURCES
        "Src/*.c"
        "Src/msp/*.c"
        "./startup_stm32f401xc.s"
)

file(GLOB LIB_SOURCES
        "lib/CMSIS/Device/ST/STM32F4xx/Source/*.c"
        "lib/HAL/Src/*.c"
        "lib/FreeRTOS/Src/*.c"
        "lib/FreeRTOS_Port/Src/*.c"
        "lib/INA3221/Src/*.c"
        "lib/shared_values/Src/*.c"
        "lib/logger/Src/*.c"
        "lib/ssd1306/Src/*.c"
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        ${APP_SOURCES}
        ${LIB_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Include
        lib/CMSIS/Include
        lib/CMSIS/Device/ST/STM32F4xx/Include
        lib/HAL/Include
        lib/FreeRTOS_Port/Include
        lib/FreeRTOS/Include
        lib/INA3221/Include
        lib/shared_values/Include
        lib/logger/Include
        lib/ssd1306/Include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        STM32F401xC
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Enable float formatting in printf like methods
target_link_options(stm32_f401_cbu6 PRIVATE -u _printf_float)