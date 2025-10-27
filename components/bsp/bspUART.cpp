#include "bspUART.h"

// std namespace statement of use
using namespace std;


/// @brief Serial port initialize
/// @param px : Serial port instance
void vbspUARTInit(UartDmaDriver * px) 
{
    // check param
    if(nullptr == px)
    {
        panic("ERROR: UART driver faild\n");
        return;
    }

    // Initialize the driver
    if (px->init() != UartDmaDriver::SUCCESS) {
        panic("ERROR: UART driver initialization failed\n");
        return;
    }
    
    // Start UART operations
    if (px->start() != UartDmaDriver::SUCCESS) {
        panic("ERROR: UART driver start failed\n");
        return;
    }
}
 
/// @brief read some bytes from uart
/// @param px : Serial port instance
/// @param puc : buffer pointer
/// @param lMaxSize : the max size of read
/// @return read size
int lbspUARTRead(UartDmaDriver * px, uint8_t * puc, int lMaxSize)
{
    int rsize = 0;
    // Event and data buffers
    UartDmaDriver::Event event;
    // Wait for next event - yields CPU until data arrives
    if (!px->popEvent(&event, portMAX_DELAY)) {
        return (0U); // Timeout (shouldn't happen with portMAX_DELAY)
    }
    // deal with even
    switch (event.type) 
    {
        case UartDmaDriver::EVT_DATA: { // Data is available - read it from internal buffer
            size_t to_read = std::min(event.size, (size_t)lMaxSize);
            size_t bytes_read = px->read(puc, to_read);
            if (bytes_read > 0) {
                // copy data to stream
                rsize = bytes_read;
            }
            break;
        }
        
        case UartDmaDriver::EVT_OVERFLOW: { // Handle buffer overflow: drain it to recover
            // Step 1: Drain all pending events from the queue
            UartDmaDriver::Event drain_event;
            while (px->popEvent(&drain_event, 0)) {
                // Discard overflow and old data events
                if (drain_event.type == UartDmaDriver::EVT_DATA) {
                    px->read(puc, lMaxSize);
                }
            }
            
            // Step 2: Drain the ring buffer until it's empty
            size_t total_discarded = 0;
            size_t bytes_read;
            do {
                bytes_read = px->read(puc, lMaxSize);
                total_discarded += bytes_read;
            } while (bytes_read == lMaxSize); // Continue until we read the last chunk of data
            // overflow flag
            rsize = -1;
            break;
        }
    }

    return rsize;
}

/// @brief transmit some bytes from uart
/// @param px : Serial port instance
/// @param puc : buffer pointer
/// @param lSize : transmit size
/// @return no used
int lbspUARTWrite(UartDmaDriver * px, uint8_t * puc, int lSize)
{
    return (int)px->send(puc, lSize);
}