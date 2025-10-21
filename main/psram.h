#ifndef PSRAM_H_
#define PSRAM_H_

#include "rp2350.h"

// psram
#define PSRAM_BASE      (0x11000000u)           // psram address base
#define PSRAM_SIZE      (8 * 1024 * 1024)       // psram size (byte)
#define PSRAM_CSI_PIN   19                      // psram chip select pin

// address allocation
// uart rx buffer
#define UART_RX_BUFFER_BASE     PSRAM_BASE
#define UART_RX_BUFFER_SIZE     UART_RX_BUFFER_BYTES_ALL_BLOCK


#endif /* PSRAM_H_ */
