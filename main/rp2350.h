#ifndef RP2350_H_
#define RP2350_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/structs/qmi.h"
#include "sfe_psram.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "hardware/dma.h"
#include "bsp.h"
#include "psram.h"
#include "UartDmaDriver.hpp"
#include <cstdio>
#include <algorithm>
#include "pico/unique_id.h"
#include "FreeRTOS_CLI.h"
#include "cli/cli.hpp"
#include "serial.h"

#ifdef __cplusplus
extern "C" {
#endif

// UART configuration - adapt these for your hardware
#define UART_CLI            uart0
#define UART_CLI_TX_PIN     PICO_DEFAULT_UART_TX_PIN
#define UART_CLI_RX_PIN     PICO_DEFAULT_UART_RX_PIN
#define UART_CLIBAUD_RATE   115200


// record psram size
extern size_t psram_size;


#ifdef __cplusplus
}
#endif

#endif /* RP2350_H_ */