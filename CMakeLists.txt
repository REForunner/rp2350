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

# New addition: Automatically initialize and update all Git submodules.
find_package(Git QUIET)
if(GIT_FOUND)
    # 1. Check if the .gitmodules file exists (to confirm that the project has submodule configuration)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.gitmodules")
        message(STATUS "Found .gitmodules; checking Git submodules...")
        
        # 2. execute git submodule update --init --recursive：
        #    - init：Initialize the unregistered sub-module
        #    - recursive：Recursive update of nested sub-modules (such as sub-modules that the sub-modules depend on)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}  # Project root directory
            RESULT_VARIABLE _git_submod_result          # Command return code
            OUTPUT_VARIABLE _git_submod_out             # standard output
            ERROR_VARIABLE _git_submod_err              # Error output
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        
        # 3. Check the execution result of the command
        if(_git_submod_result EQUAL 0)
            message(STATUS "All Git submodules updated successfully.")
        else()
            # Non-fatal error: Prompt to manually execute the command (to prevent CMake from failing directly)
            message(WARNING 
                "Failed to update Git submodules (return code: ${_git_submod_result}).\n"
                "Standard Output: ${_git_submod_out}\n"
                "Error Output: ${_git_submod_err}\n"
                "Please manually run this command in project root: \n"
                "  git submodule update --init --recursive"
            )
        endif()
    else()
        message(STATUS "No .gitmodules file found; no Git submodules to update.")
    endif()
else()
    # Git not found: Prompting user to install Git
    message(WARNING 
        "Git is not installed on your system.\n"
        "Cannot auto-update Git submodules. Please install Git first, then run:\n"
        "  git submodule update --init --recursive"
    )
endif()

# set freertos path
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel CACHE PATH "")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rp2350 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rp2350)

pico_set_program_name(rp2350 "rp2350")
pico_set_program_version(rp2350 "0.1")

# set uart parameter
# Optional: Configure driver parameters before adding subdirectory
set(UART_RX_MAX_BAUDRATE 921600)       # Max baudrate (default: 921600)
set(UART_DMA_RING_BITS 14)             # DMA buffer size 2^13=16384 bytes (default: 11)
set(UART_RX_MAX_EVT_DATA_SIZE 1024)    # Max event data size (default: 256)
set(UART_TX_BUFFER_SIZE 1024)          # TX buffer size (default: 256)
set(UART_WD_TICK_US 450)               # Tick time for DMA watchdog[us] (default: 450)
set(UART_WD_SILENCE_TICKS 4)           # The number of ticks to consider silence (default: 4)

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
        # tinyusb_device  # tinyusb
        # tinyusb_board
        )

# Add the standard include files to the build
target_include_directories(rp2350 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(rp2350)

