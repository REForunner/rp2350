# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# set freertos path
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel CACHE PATH "")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rp2350 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rp2350 main/rp2350.cpp)

pico_set_program_name(rp2350 "rp2350")
pico_set_program_version(rp2350 "0.1")

# set uart parameter
# Optional: Configure driver parameters before adding subdirectory
set(UART_RX_MAX_BAUDRATE 921600)       # Max baudrate (default: 921600)
set(UART_DMA_RING_BITS 13)             # DMA buffer size 2^13=8192 bytes (default: 11)
set(UART_RX_MAX_EVT_DATA_SIZE 1024)    # Max event data size (default: 256)
set(UART_TX_BUFFER_SIZE 1024)          # TX buffer size (default: 256)

# add subdirectory.
add_subdirectory(main)                      # main
add_subdirectory(app)                       # applocation
add_subdirectory(components/sparkfun_pico)  # psram
add_subdirectory(components/freertos)       # operation system
add_subdirectory(components/log.c)          # log
add_subdirectory(components/bsp)            # board support package
add_subdirectory(lib/UartDmaDriver)         # UartDmaDriver

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(rp2350 1)
pico_enable_stdio_usb(rp2350 0)

# Add the standard library to the build
target_link_libraries(rp2350 PRIVATE
        pico_stdlib
        main            # main library
        app             # applocation
        sparkfun_pico   # psram
        freertos        # freertos-kernel
        log             # log.c
        bsp             # board support package
        UartDmaDriver   # UartDmaDriver
        pico_unique_id  # unique id
        )

# Add the standard include files to the build
target_include_directories(rp2350 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(rp2350)

