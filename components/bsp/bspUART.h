#ifndef BSP_UART_H_
#define BSP_UART_H_

#include "bsp.h"
#include <algorithm>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"


// /// @brief Serial port initialize
// /// @param px : Serial port instance
// void vbspUARTInit(UartDmaDriver * px);

// /// @brief read some bytes from uart
// /// @param px : Serial port instance
// /// @param puc : buffer pointer
// /// @param lMaxSize : the max size of read
// /// @return read size
// int lbspUARTRead(UartDmaDriver * px, uint8_t * puc, int lMaxSize);

// /// @brief transmit some bytes from uart
// /// @param px : Serial port instance
// /// @param puc : buffer pointer
// /// @param lSize : transmit size
// /// @return no used
// int lbspUARTWrite(UartDmaDriver * px, uint8_t * puc, int lSize);


#endif /* BSP_UART_H_ */