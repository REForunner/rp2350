#ifndef bsp_Uart_H_
#define bsp_Uart_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"

// uart irq priority
#define UART_IRQ_PRIORITY               5U
// dma uart channel irq priority
#define UART_DMA_CHANNEL_IRQ_PRIORITY   6U
// receive buffer size
#define UART_RX_BUFFER_BYTES_ALL_BLOCK  2048U
// How many times must it be received to fill the buffer used DMA
#define UART_RXBUFF_BLOCK_NUM           2U

typedef struct UartHandler_t
{
    uart_inst_t * uart;     ///> UART instance. uart0 or uart1.
    uint txGPIO;            ///> gpio by tx used
    uint rxGPIO;            ///> gpio by rx used
    uint TxChannel;         ///> tx channel, when the dma is used.
    uint RxChannel;         ///> rx channel, when the dma is used.
    uint baudrate;          ///> Actual set baudrate.
    uint irq_num;           ///> Interrupt number of DMA.
    irq_handler_t TXdmaCb;  ///> Interrupt service function of DMA(uart tx).
    irq_handler_t RXdmaCb;  ///> Interrupt service function of DMA(uart rx).
    irq_handler_t uartCb;   ///> Interrupt service function of UART.
    uint8_t pucBuffer[UART_RX_BUFFER_BYTES_ALL_BLOCK];       ///> receive buffer.
} UartHandler_t;

/// @brief Initialise a UART with DMA(tx and rx).
/// @param tx : the GPIO number of tx.
/// @param rx : the GPIO number of rx.
/// @param px : Pointer to the configuration result structure.
void vbspUartInitWithDMA(uint tx, uint rx, UartHandler_t * px);

/// @brief Start a transmission by used DMA.
/// @param puc : the pointer of source data.
/// @param ul : Number bytes of transfers.
/// @param px : Pointer to the configuration structure.
static inline void vbspUartTransferWithDMA(const uint8_t * puc, uint ul, UartHandler_t * px)
{
    dma_channel_transfer_from_buffer_now(px->TxChannel, puc, ul);
}

#endif /* bsp_Uart_H_ */