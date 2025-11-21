#include "rp2350.h"


/*-----------------------------------------------------------*/

// cpu run time statistics time base
#ifndef CPU_RUN_TIME_BASE_US
    #define CPU_RUN_TIME_BASE_US    500
#endif  /* CPU_RUN_TIME_BASE_US */

/*-----------------------------------------------------------*/

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

// inside heap
static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
// Define two non-contiguous memory areas
const HeapRegion_t xHeapRegions[] = {
    { (uint8_t*)ucHeap, configTOTAL_HEAP_SIZE },    // area 1：inside
    { (uint8_t*)PSRAM_BASE, PSRAM_SIZE },           // area 2：psram
    { NULL, 0 }                                     // end
};

// stearm buffer used by cdc
static StreamBufferHandle_t cdc_rx_streambuf[CFG_TUD_CDC];

/*-----------------------------------------------------------*/

// cli use cdc 0
#define CLI_USB_CDC_NUMBER       0

// cli data interface
static int lCLIRead(uint8_t * puc, int lMaxSize) 
{
    // receive data from steam-buffer
    return (int)xStreamBufferReceive(cdc_rx_streambuf[CLI_USB_CDC_NUMBER], puc, lMaxSize, portMAX_DELAY);   
}
static int lCLIWrite(uint8_t * puc, int lMaxSize) 
{ 
    // use cdc 0
    tud_cdc_n_write(CLI_USB_CDC_NUMBER, (uint8_t const *)puc, lMaxSize);
    tud_cdc_n_write_flush(CLI_USB_CDC_NUMBER);
    return 0; 
}

static const cli_t xCLIInterface =
{
    .r = lCLIRead,
    .w = lCLIWrite,
};

/*-----------------------------------------------------------*/

#if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) )
    // cpu run time statistics
    struct repeating_timer xTimer;
    static bool prvCPURunTimeStatistic(__unused struct repeating_timer *t) { ulHighFrequencyTimerTicks += 1U; return true; }
#endif /* ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) ) */

/*-----------------------------------------------------------*/

static void prvusbThread(void * pv)
{
#if ( CFG_TUSB_OS != OPT_OS_FREERTOS )
    TickType_t wake;
    wake = xTaskGetTickCount();
#endif
    while (true)
    {
        // TinyUSB device task | must be called regurlarly
        tud_task();
#if ( CFG_TUSB_OS != OPT_OS_FREERTOS )
        // If suspended or disconnected, delay for 1ms
        if (tud_suspended() || !tud_connected())
            xTaskDelayUntil(&wake, pdMS_TO_TICKS(3));
        // Go to sleep for up to a tick if nothing to do
        else if (!tud_task_event_ready())
            xTaskDelayUntil(&wake, pdMS_TO_TICKS(3));
#endif
    }
}

static void prvcdc(void * p)
{
    char hello[] = "hello world!!!\r\n";
    while(true)
    {
        tud_cdc_n_write(1, (uint8_t const *)hello, strlen(hello));
        tud_cdc_n_write_flush(1);
        vTaskDelay(1000);
    }
}

// callback when data is received on a CDC interface
void tud_cdc_rx_cb(uint8_t itf)
{
    // allocate buffer for the data in the stack
    uint8_t buf[CFG_TUD_CDC_RX_BUFSIZE];
    // read the available data 
    uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));
    // check if the data was received 0 bytes
    if(count == 0)
        return;
    // copy data to steam-buffer
    // To ensure that the USB time is processed promptly
    // any excess data is discarded.
    if(count > xStreamBufferSend(cdc_rx_streambuf[itf], buf, count, 0))
    {
        char overflow[] = "The cdc data is overflow used by steam-buffer!";
        // overflow warning
        tud_cdc_n_write(itf, (uint8_t const *)overflow, strlen(overflow));
        tud_cdc_n_write_flush(itf);
    }
}

/*-----------------------------------------------------------*/

// entry
int main(void)
{
    // Initialize TinyUSB stack
    tusb_init();

    // let pico sdk use the first cdc interface for std io
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
    // creat steam-buffer, used by cdc
    for(int i = 0; i < CFG_TUD_CDC; i++)
    {
        cdc_rx_streambuf[i] = xStreamBufferCreate(CFG_TUD_CDC_RX_BUFSIZE, 1);
    }
    // enable trace recorder
        
    /* Create that task that handles the console itself. */
    // xTaskCreate( vLCDTask, "lcd", 1024U, (void *)&xLCDdriver, 2, &xLCDHandle );

    // usb thread
    xTaskCreateAffinitySet(prvusbThread, "tud", 4096UL, NULL, 15, CORE_NUMBER(0), NULL);
    xTaskCreate(prvcdc, "cdc", configMINIMAL_STACK_SIZE, NULL, 3, NULL);

    // Create the command line task
    xCLIStart( (void * const)&xCLIInterface, NULL, 3 );
    
    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Never reached
    while (true) 
    {
        printf("FATAL: Scheduler returned!\n");
        sleep_ms(1000);
    }

    return 0;
}

/*-----------------------------------------------------------*/