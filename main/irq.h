#ifndef IRQ_H_
#define IRQ_H_

#include "bsp.h"

extern UartHandler_t * uHandler;

/// @brief uart Interrupt service
/// @param void 
void virqUartServe(void);

/// @brief UART TX dma Interrupt service
/// @param void 
void virqTXDMAServe(void);

/// @brief UART RX dma Interrupt service
/// @param void 
void virqRXDMAServe(void);

#endif /* IRQ_H_ */