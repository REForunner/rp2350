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
#include "hardware/pio.h"
#include "rp2350.pio.h"
#include "lcd/lcd.hpp"


#ifdef __cplusplus
extern "C" {
#endif


#if (__STDC_VERSION__ >= 201112L)
#define STATIC_ASSERT(COND, MSG) _Static_assert(COND, MSG)
#else
#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define UNIQUE_STATIC_ASSERT_ID TOKENPASTE2(_static_assertion_, __LINE__)

#define STATIC_ASSERT(COND, MSG) \
    typedef char __attribute__((unused)) UNIQUE_STATIC_ASSERT_ID[(COND) ? 1 : -1]
#endif // (__STDC_VERSION__ >= 201112L)


/* lcd default display direction */
#define SCREEN_DISPLAY_DIR_DEF      Horizontal
/* colour gamut */
#define SCREEN_COLOUR_GAMUT         565

#if (SCREEN_COLOUR_GAMUT == 444)
    #define COLOUR_GAMUT_BYTES      2
#elif (SCREEN_COLOUR_GAMUT == 565)
    #define COLOUR_GAMUT_BYTES      2
#elif (SCREEN_COLOUR_GAMUT == 666)
    #define COLOUR_GAMUT_BYTES      4
#elif (SCREEN_COLOUR_GAMUT == 888)
    #define COLOUR_GAMUT_BYTES      4
#else
    #error "Please select a color format!!!"
#endif

/* Screen pixel size */
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   135
/* screen total buffer size */
#define SCREEN_TOTAL_BUFFER_SIZE    (SCREEN_WIDTH * SCREEN_HEIGHT * COLOUR_GAMUT_BYTES)


/* Used for LCD and SD Card */
#define SPI_LCD_SDIO    spi1
/* lcd data/command switch pin */
#define SPI_LCD_DC_PIN  8
/* lcd back-light control pin */
#define SPI_LCD_BL_PIN  25
/* lcd spi cs pin */
#define SPI_CS_LCD_PIN  9
/* sd card spi cs pin */
#define SPI_CS_SD_PIN   15
/* spi clk pin */
#define SPI_CLK_PIN     10
/* spi mosi pin */
#define SPI_MOSI_PIN    11
/* spi miso pin */
#define SPI_MISO_PIN    12
/* spi speed(baud rate) */
#define SPI_BAUDRATE    25000000


/* lcd dc pin switch to cmd */
#define LCD_DC_PIN_CMD()        gpio_put(SPI_LCD_DC_PIN, 0)
/* lcd dc pin switch to data */
#define LCD_DC_PIN_DATA()       gpio_put(SPI_LCD_DC_PIN, 1)

/* lcd enable */
#define LCD_CS_PIN_ENABLE()     gpio_put(SPI_CS_LCD_PIN, 0)
/* lcd disable */
#define LCD_CS_PIN_DISABLE()    gpio_put(SPI_CS_LCD_PIN, 1)

/* sd enable */
#define SD_CS_PIN_ENABLE()      gpio_put(SPI_CS_SD_PIN, 0)
/* sd disable */
#define SD_CS_PIN_DISABLE()     gpio_put(SPI_CS_SD_PIN, 1)

/* lcd back-light on */
#define LCD_BL_PIN_ON()         gpio_put(SPI_LCD_BL_PIN, 0)
/* lcd back-light off */
#define LCD_BL_PIN_OFF()        gpio_put(SPI_LCD_BL_PIN, 1)


// PIO config
#define LCD_PIO                     0
#define LCD_PIO_CLK_DIV             10.0f
#define LCD_PROBE_SIDE_PIN_BASE     SPI_LCD_DC_PIN                 // gpio 8
#define LCD_PROBE_PIN_DC            (LCD_PROBE_SIDE_PIN_BASE + 0)  // gpio 8
#define LCD_PROBE_PIN_CS            (LCD_PROBE_SIDE_PIN_BASE + 1)  // gpio 9
#define LCD_PROBE_PIN_CLK           (LCD_PROBE_SIDE_PIN_BASE + 2)  // gpio 10
#define LCD_PROBE_PIN_MOSI          (LCD_PROBE_SIDE_PIN_BASE + 3)  // gpio 11


// UART configuration - adapt these for your hardware
#define UART_CLI            uart0
#define UART_CLI_TX_PIN     PICO_DEFAULT_UART_TX_PIN
#define UART_CLI_RX_PIN     PICO_DEFAULT_UART_RX_PIN
#define UART_CLIBAUD_RATE   115200


// record psram size
extern size_t xPsramSize;
// extern : Define two non-contiguous memory areas
extern const HeapRegion_t xHeapRegions[];
// lcd task handle
extern TaskHandle_t xLCDHandle;
// lcd driver
extern struct lcd_t xLCDdriver;


#ifdef __cplusplus
}
#endif

#endif /* RP2350_H_ */