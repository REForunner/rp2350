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
#include "irq.h"
#include "rp2350.h"

// uart hardware config
#define UART_CUSTOM     uart0
#define UART_TX_GPIO    PICO_DEFAULT_UART_TX_PIN
#define UART_RX_GPIO    PICO_DEFAULT_UART_RX_PIN
#define UART_BAUDRATE   115200

#endif /* RP2350_H_ */