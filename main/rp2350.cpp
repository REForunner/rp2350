#include "rp2350.h"


// cpu run time statistics time base
#ifndef CPU_RUN_TIME_BASE_US
    #define CPU_RUN_TIME_BASE_US    500
#endif  /* CPU_RUN_TIME_BASE_US */


// lcd driver
lcd_t xLCDdriver = 
{ 
    .pio = LCD_PIO,                 /* default pio */
    .SidePinBase = SPI_LCD_DC_PIN,  /* side-set pin base gpui number */
    .OutPin = SPI_MOSI_PIN,         /* out pin base gpio mumber */
    .BackLightPin = SPI_LCD_BL_PIN, /* back light control gpio */
    .div = LCD_PIO_CLK_DIV,         /* pio Frequency division (current 1.0f = 75MHz) */
    .dir = SCREEN_DISPLAY_DIR_DEF,  /* display default direction */
    .width = SCREEN_WIDTH,
    .height = SCREEN_HEIGHT 
};
// lcd task handle
TaskHandle_t xLCDHandle = NULL;
// record psram size
size_t xPsramSize = 0UL;
// Create driver instance
UartDmaDriver uart_driver(UART_CLI, UART_CLI_TX_PIN, UART_CLI_RX_PIN, UART_CLIBAUD_RATE);
// inside heap
static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
// Define two non-contiguous memory areas
const HeapRegion_t xHeapRegions[] = {
    { (uint8_t*)ucHeap, configTOTAL_HEAP_SIZE },    // area 1：inside
    { (uint8_t*)PSRAM_BASE, PSRAM_SIZE },           // area 2：psram
    { NULL, 0 }                                     // end
};

// cli data interface
static int lCLIRead(uint8_t * puc, int lMaxSize) { return lbspUARTRead(&uart_driver, puc, lMaxSize); }
static int lCLIWrite(uint8_t * puc, int lMaxSize) { return lbspUARTWrite(&uart_driver, puc, lMaxSize); }

static const cli_t xCLIInterface =
{
    .r = lCLIRead,
    .w = lCLIWrite,
};

#if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) )
    // cpu run time statistics
    struct repeating_timer xTimer;
    static bool prvCPURunTimeStatistic(__unused struct repeating_timer *t) { ulHighFrequencyTimerTicks += 1U; return true; }
#endif /* ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) ) */

// entry
int main(void)
{
    stdio_init_all();

    // get unique id and format it
    vSerialInit();

#if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) )
    // creat a repeating timer that used by cpu time statistics
    add_repeating_timer_us(CPU_RUN_TIME_BASE_US, prvCPURunTimeStatistic, NULL, &xTimer);
#endif /* ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) ) */

    // Initialise PSRAM and get the psram size
    xPsramSize = sfe_setup_psram(PSRAM_CSI_PIN);
    // PSRAM Initialise success, zero clearing
    if(0UL != xPsramSize)
    {
        // memset((void *)PSRAM_BASE, 0x00U, xPsramSize);
    }
    else
    {
        panic("PSRAM initialization failure!!!!!\n");
    }
    
    // Initialize heap5 and manage the defined memory areas
    // It must be called after the Initialise success is completed.
    vPortDefineHeapRegions(xHeapRegions);

    // Initialise uart0
    vbspUARTInit(&uart_driver); 
    
    // enable trace recorder
        
    /* Create that task that handles the console itself. */
    xTaskCreate( vLCDTask, "lcd", 1024U, (void *)&xLCDdriver, configTIMER_TASK_PRIORITY - 2, &xLCDHandle );

    // Create the command line task
    xCLIStart( (void * const)&xCLIInterface, NULL );
    
    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Never reached
    panic("FATAL: Scheduler returned!\n");
    while (true) sleep_ms(1000);

    return 0;
}