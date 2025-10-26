#ifndef PSRAM_H_
#define PSRAM_H_

#include "rp2350.h"

// psram
#define PSRAM_BASE      (0x11000000u)           // psram address base
#define PSRAM_SIZE      (8 * 1024 * 1024)       // psram size (byte)
#define PSRAM_CSI_PIN   19                      // psram chip select pin

#endif /* PSRAM_H_ */
