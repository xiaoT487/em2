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
set(CMAKE_PROJECT_NAME miao1)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        user_config/bsp/ws2812/ws2812.h
        user_config/bsp/ws2812/ws2812.c
        user_config/bsp/buzzer/buzzer.c
        user_config/bsp/buzzer/buzzer.h
        user_config/bsp/usart/uart.c
        user_config/bsp/usart/uart.h
        user_config/bsp/usart/buffer.c
        user_config/bsp/can/user_can.c
        user_config/bsp/can/user_can.h
        user_config/bsp/imu/bmi088/BMI088driver.c
        user_config/bsp/imu/bmi088/BMI088driver.h
        user_config/bsp/imu/bmi088/BMI088Middleware.c
        user_config/bsp/imu/bmi088/BMI088Middleware.h
        user_config/bsp/imu/bmi088/BMI088reg.h
        user_config/bsp/imu/imu.c
        user_config/bsp/imu/imu.h
        user_config/app/freeRTOS/task_status/task_status.c
        user_config/app/freeRTOS/task_status/task_status.h
        user_config/app/freeRTOS/Communication/communication.c
        user_config/app/freeRTOS/Communication/communication.h
        user_config/app/PID/PID.c
        user_config/app/PID/PID.h
        user_config/app/ins/ins.c
        user_config/app/ins/ins.h
        user_config/components/motor/dji/dji_motor.c
        user_config/components/motor/dji/dji_motor.h
        user_config/app/freeRTOS/motor_control/motor_ctrl.c
        user_config/app/freeRTOS/motor_control/motor_ctrl.h
        user_config/components/motor/dm/dm_motor.c
        user_config/components/motor/dm/dm_motor.h
        user_config/components/remote/remote.c
        user_config/components/remote/remote.h
)


# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        user_config/bsp/ws2812/ws2812.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        user_config
        user_config/bsp/ws2812
        user_config/app
        user_config/app/ins
        user_config/bsp/imu
        user_config/bsp/usart
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
