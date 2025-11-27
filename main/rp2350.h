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
#include "hardware/dma.h"
#include "bsp.h"
#include "psram.h"
#include "pico/unique_id.h"
#include "FreeRTOS_CLI.h"
#include "cli/cli.hpp"
#include "serial.h"
#include "hardware/pio.h"
#include "rp2350.pio.h"
#include "lcd/lcd.hpp"
#include <tusb.h>
#include "pico/multicore.h"
#include "stream_buffer.h"
#include "dap/DAP_config.h"
#include "dap/DAP.h"


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

#ifndef USE_PROBE
#define USE_PROBE   0
#endif

#if USE_PROBE
#define probe_info(format,args...) \
do { \
	vTaskSuspendAll(); \
	printf(format, ## args); \
	xTaskResumeAll(); \
} while (0)
#else
#define probe_info(format,...) ((void)0)
#endif

#if USE_PROBE
#define probe_debug(format,args...) \
do { \
	vTaskSuspendAll(); \
	printf(format, ## args); \
	xTaskResumeAll(); \
} while (0)
#else
#define probe_debug(format,...) ((void)0)
#endif

#if USE_PROBE
#define probe_dump(format,args...)\
do { \
	vTaskSuspendAll(); \
	printf(format, ## args); \
	xTaskResumeAll(); \
} while (0)
#else
#define probe_dump(format,...) ((void)0)
#endif

/* some task priority */
#define TUD_TASK_PRIO  	(tskIDLE_PRIORITY + 3)
#define DAP_TASK_PRIO  	(tskIDLE_PRIORITY + 2)
#define CLI_TASK_PRIO	(tskIDLE_PRIORITY + 2)

/* swd pin */
// #define SWCLK_PIN   22	// in top cmakelists.txt
// #define SWDIO_PIN   23
// swdio interface config
// PIO config
#define PROBE_SM 			0
// set swd pin base
#define PROBE_PIN_OFFSET 	SWCLK_PIN	// swclk = pinBase + 0;	swdio = pinBase + 1


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
#define LCD_PIO_CLK_DIV             1.0f
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


// core number
#define CORE_NUMBER(num)    (1 << num)

// record psram size
extern size_t xPsramSize;
// extern : Define two non-contiguous memory areas
extern const HeapRegion_t xHeapRegions[];
// lcd task handle
extern TaskHandle_t xLCDHandle;
// lcd driver
extern struct lcd_t xLCDdriver;
// task handle
extern TaskHandle_t dap_taskhandle, tud_taskhandle;
// dap thread
extern void dap_thread(void *ptr);
// stearm buffer used by dap thread
extern StreamBufferHandle_t dapStreambuf;


#ifdef __cplusplus
}
#endif

#endif /* RP2350_H_ */