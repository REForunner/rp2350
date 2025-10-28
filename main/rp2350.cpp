#include "rp2350.h"

// record psram size
size_t psram_size = 0UL;
// Create driver instance
UartDmaDriver uart_driver(UART_CLI, UART_CLI_TX_PIN, UART_CLI_RX_PIN, UART_CLIBAUD_RATE);

// cli data interface
static int lCLIRead(uint8_t * puc, int lMaxSize) { return lbspUARTRead(&uart_driver, puc, lMaxSize); }
static int lCLIWrite(uint8_t * puc, int lMaxSize) { return lbspUARTWrite(&uart_driver, puc, lMaxSize); }

static const cli_t xCLIInterface =
{
    .r = lCLIRead,
    .w = lCLIWrite,
};

// entry
int main(void)
{
    stdio_init_all();

    // get unique id and format it
    vSerialInit();
    
    // Initialise PSRAM and get the psram size
    psram_size = sfe_setup_psram(PSRAM_CSI_PIN);
    
    // Initialise uart0
    vbspUARTInit(&uart_driver); 
    
    // enable trace recorder

    // Create the command line task
    xCLIStart( (void * const)&xCLIInterface, NULL );
    
    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Never reached
    panic("FATAL: Scheduler returned!\n");
    while (true) sleep_ms(1000);

    return 0;
}
