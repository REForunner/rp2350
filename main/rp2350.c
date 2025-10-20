#include "rp2350.h"

// psram
#define PSRAM_BASE      (0x11000000u)           // psram address base
#define PSRAM_SIZE      (8 * 1024 * 1024)       // psram size (byte)
#define PSRAM_CSI_PIN   19                      // psram chip select pin

UartHandler_t * uHandler = NULL;

// entry
int main()
{
    stdio_init_all();

    // Initialise PSRAM and get the psram size
    size_t psram_size = sfe_setup_psram(PSRAM_CSI_PIN);
    
    // initialise uart and used dma
    uHandler = (UartHandler_t *)pvPortCalloc(1, sizeof(UartHandler_t));
    if(uHandler != NULL)
    {
        uHandler->uart = UART_CUSTOM;
        uHandler->baudrate = UART_BAUDRATE;
        uHandler->TXdmaCb = (irq_handler_t)virqTXDMAServe;
        uHandler->RXdmaCb = (irq_handler_t)virqRXDMAServe;
        uHandler->uartCb = (irq_handler_t)virqUartServe;

        vbspUartInitWithDMA(UART_TX_GPIO, UART_RX_GPIO, uHandler);
    }
    else
    {
        printf("calloc error.\n");
    }

    static char h[] = "hello world!!!";
    vbspUartTransferWithDMA((const uint8_t *)h, sizeof(h), uHandler);
    while(1) sleep_ms(1000);
    // // Launch main task
    // xTaskCreate(
    //     /*pxTaskCode*/    mainTask, 
    //     /*pcName*/        "mainTask", 
    //     /*uxStackDepth*/  512,
    //     /*pvParameters*/  NULL, 
    //     /*uxPriority*/    tskIDLE_PRIORITY + 1, 
    //     /*pxCreatedTask*/ NULL
    // );
    
    // xTaskCreate(
    //     /*pxTaskCode*/    mainTask2, 
    //     /*pcName*/        "mainTask2", 
    //     /*uxStackDepth*/  512,
    //     /*pvParameters*/  NULL, 
    //     /*uxPriority*/    tskIDLE_PRIORITY + 1, 
    //     /*pxCreatedTask*/ NULL
    // );

    // // Start the FreeRTOS scheduler
    // vTaskStartScheduler();
    
    // Never reached
    printf("FATAL: Scheduler returned!\n");
    while (true) sleep_ms(1000);

    return 0;
}
