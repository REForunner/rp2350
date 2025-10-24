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

// uart hardware config
#define UART_CUSTOM     uart0
#define UART_TX_GPIO    PICO_DEFAULT_UART_TX_PIN
#define UART_RX_GPIO    PICO_DEFAULT_UART_RX_PIN
#define UART_BAUDRATE   115200
// receive buffer size
#define UART_RX_BUFFER_BYTES_ALL_BLOCK  0x4000U
// How many times must it be received to fill the buffer used DMA
#define UART_RXBUFF_BLOCK_NUM           0x20U


#endif /* RP2350_H_ */