#include "rp2350.h"

// Create driver instance
UartDmaDriver uart_cli_driver(UART_CLI, UART_CLI_TX_PIN, UART_CLI_RX_PIN, UART_CLIBAUD_RATE);

void uart_processing_task(void* param) {
    // Initialize the driver
    if (uart_cli_driver.init() != UartDmaDriver::SUCCESS) {
        printf("ERROR: UART driver initialization failed\n");
        vTaskDelete(NULL);
        return;
    }
    
    // Start UART operations
    if (uart_cli_driver.start() != UartDmaDriver::SUCCESS) {
        printf("ERROR: UART driver start failed\n");
        vTaskDelete(NULL);
        return;
    }
    
    printf("UART driver started successfully on pins TX=%d, RX=%d\n", 
           UART_CLI_TX_PIN, UART_CLI_RX_PIN);
    
    // Event and data buffers
    UartDmaDriver::Event event;
    uint8_t buffer[UART_RX_MAX_EVT_DATA_SIZE];  // Match max event data size
    
    while (true) {
        // Wait for next event - yields CPU until data arrives
        if (!uart_cli_driver.popEvent(&event, portMAX_DELAY)) {
            continue; // Timeout (shouldn't happen with portMAX_DELAY)
        }
        
        switch (event.type) {
            case UartDmaDriver::EVT_DATA: { // Data is available - read it from internal buffer
                size_t to_read = std::min(event.size, sizeof(buffer));
                size_t bytes_read = uart_cli_driver.read(buffer, to_read);
                if (bytes_read > 0) {
                    printf("Received %zu bytes", bytes_read);
                    if (event.silenceFlag) {
                        printf(" (frame end detected)");
                    }
                    printf("\n");
                    
                    // Process your data here...
                    // For example, echo it back:
                    // uart_cli_driver.send(buffer, bytes_read);
                }
                break;
            }
            
            case UartDmaDriver::EVT_OVERFLOW: { // Handle buffer overflow: drain it to recover
                printf("WARNING: UART buffer overflow - starting recovery\n");
                
                // Step 1: Drain all pending events from the queue
                UartDmaDriver::Event drain_event;
                while (uart_cli_driver.popEvent(&drain_event, 0)) {
                    // Discard overflow and old data events
                    if (drain_event.type == UartDmaDriver::EVT_DATA) {
                        uart_cli_driver.read(buffer, sizeof(buffer));
                    }
                }
                
                // Step 2: Drain the ring buffer until it's empty
                size_t total_discarded = 0;
                size_t bytes_read;
                do {
                    bytes_read = uart_cli_driver.read(buffer, sizeof(buffer));
                    total_discarded += bytes_read;
                } while (bytes_read == sizeof(buffer)); // Continue until we read the last chunk of data
                
                printf("Recovery complete: discarded %zu bytes\n", total_discarded);
                printf("Consider reading data faster or reducing baud rate\n");
                break;
            }
        }

        // No need to yield here - FreeRTOS will handle task switching when looping back to `popEvent()`
    }
}

// entry
int main(void)
{
    stdio_init_all();
    
    // Initialise PSRAM and get the psram size
    size_t psram_size = sfe_setup_psram(PSRAM_CSI_PIN);
    
    printf("PSRAM size is 0x%lx.\n", psram_size);

    // get unique id
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    
    printf("Unique identifier: 0x%llx.\n", *(uint64_t *)&board_id);

    // printf("Basic UART Example Starting...\n");
    // // Create the UART processing task
    // xTaskCreate(uart_processing_task, "UART", 2048, NULL, 1, NULL);
    
    // Never reached
    printf("FATAL: Scheduler returned!\n");
    while (true) sleep_ms(1000);

    return 0;
}
