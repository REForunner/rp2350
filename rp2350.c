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

// psram
#define PSRAM_BASE      (0x11000000u)           // psram address base
#define PSRAM_SIZE      (8 * 1024 * 1024)       // psram size (byte)
#define PSRAM_CSI_PIN   19                      // psram chip select pin

void mainTask(void* param) {
    while (true) {
        printf("Hello from FreeRTOS task!\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void mainTask2(void * param)
{
    while (true) {
        printf("Hello, world!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

UartHandler_t * uHandler = NULL;

// entry
int main()
{
    stdio_init_all();

    // Initialise PSRAM and get the psram size
    size_t psram_size = sfe_setup_psram(PSRAM_CSI_PIN);

    printf("start!\n");
    uHandler = (UartHandler_t *)pvPortCalloc(1, sizeof(UartHandler_t));
    if(uHandler != NULL)
    {
        uHandler->uart = uart0;
        uHandler->baudrate = 115200U;
        
        vbspUartInitWithDMA(0, 1, uHandler);
    }
    else
    {
        printf("calloc error.\n");
    }
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
