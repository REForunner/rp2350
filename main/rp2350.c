#include "rp2350.h"

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
        // must init psram first.
        uHandler->pucBuffer = (uint8_t *)UART_RX_BUFFER_BASE;
        uHandler->bufferSize = (uint)UART_RX_BUFFER_BYTES_ALL_BLOCK;
        uHandler->bufferBlocks = (uint)UART_RXBUFF_BLOCK_NUM;

        vbspUartInitWithDMA(UART_TX_GPIO, UART_RX_GPIO, uHandler);
    }
    else
    {
        panic("calloc error.");
    }
    
    static char h[] = "hello world!!!";
    // vbspUartTransferWithDMA((const uint8_t *)h, sizeof(h), uHandler);
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
