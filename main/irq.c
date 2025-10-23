#include "irq.h"

/// @brief uart Interrupt service
/// @param void 
void virqUartServe(void)
{
    uint pending = uart_get_hw(uHandler->uart)->mis;
    // receive timeout interrupt status
    // Note: if use dma, the RXIM Will hardly trigger!!!
    // if(pending & UART_UARTMIS_RTMIS_BITS)
    // {
    //     // clear interrupt flag
    //     uart_get_hw(uHandler->uart)->icr |= UART_UARTMIS_RTMIS_BITS;
    //     // compare receive size of the once dma
    //     uint rxSizeOnce = uHandler->bufferSize / uHandler->bufferBlocks;    
    //     // update write pointer
    //     uint tran = dma_hw->ch[uHandler->RxChannel].transfer_count & ~DMA_CH0_TRANS_COUNT_MODE_BITS;
    //     uHandler->pRXw += (rxSizeOnce - tran);
    //     // record data count
    //     uHandler->usefulBuf += (rxSizeOnce - tran);
    //     // check write pointer validity
    //     if(uHandler->pRXw > (uHandler->pucBuffer + uHandler->bufferSize))
    //         panic("Illegal pointer writing of the RX ring buffer from RX UART Interrupt!!!");
    //     // address reset
    //     if(uHandler->pRXw == (uHandler->pucBuffer + uHandler->bufferSize))
    //         uHandler->pRXw = uHandler->pucBuffer;
        
    //     // is the ring buffer almost full???
    //     if((int)abs(uHandler->pRXr - uHandler->pRXw) < rxSizeOnce && uHandler->pRXr >= uHandler->pRXw)
    //     {
    //         // the ring buffer is almost full!!!

    //     }

    //     // There are currently no data to receive. do some deal with

    // }

    // error occured
    if(pending & UART_UARTMIS_FEMIS_BITS ||    // framing error
        pending & UART_UARTMIS_PEMIS_BITS ||   // parity error
        pending & UART_UARTMIS_BEMIS_BITS ||   // break error
        pending & UART_UARTMIS_OEMIS_BITS)     // overrun error
    {
        // clear interrupt flag
        uart_get_hw(uHandler->uart)->icr = (UART_UARTMIS_FEMIS_BITS | UART_UARTMIS_PEMIS_BITS | 
                                                UART_UARTMIS_BEMIS_BITS | UART_UARTMIS_OEMIS_BITS);
        
        panic("UART TX error was happened!!!");
    }
}

/// @brief UART TX dma Interrupt service
/// @param void 
void virqTXDMAServe(void)
{
    bool t = false;
    // tigger source
    if(DMA_IRQ_0 == uHandler->irq_num)
        t = dma_channel_get_irq0_status(uHandler->TxChannel);
    if(DMA_IRQ_1 == uHandler->irq_num)
        t = dma_channel_get_irq1_status(uHandler->TxChannel);
    if(false == t)
        return;

    // check error occur?
    if(dma_channel_hw_addr(uHandler->TxChannel)->ctrl_trig & 0x80000000U)
        panic("Read or Write error has occured from UART TX DMA Channel!!!");

    // tx dma transmit complete
    static int ll;
    ll += 1;
    
    // Clear the interrupt request.
    if(DMA_IRQ_0 == uHandler->irq_num)
        dma_hw->ints0 |= 1u << uHandler->TxChannel;
    if(DMA_IRQ_1 == uHandler->irq_num)
        dma_hw->ints1 |= 1u << uHandler->TxChannel;
}

/// @brief UART RX dma Interrupt service
/// @param void 
void virqRXDMAServe(void)
{
    bool t = false;
    // tigger source
    if(DMA_IRQ_0 == uHandler->irq_num)
        t = dma_channel_get_irq0_status(uHandler->RxChannel);
    if(DMA_IRQ_1 == uHandler->irq_num)
        t = dma_channel_get_irq1_status(uHandler->RxChannel);
    if(false == t)
        return;
    
    // check error occur?
    if(dma_channel_hw_addr(uHandler->RxChannel)->ctrl_trig & 0x80000000U)   // ahb_error
        panic("Read or Write error has occured from UART RX DMA Channel!!!");
    
    // rx dma receives 1/uHandler->bufferBlocks of buffer size datas.
    // compare receive size of the once dma
    int rxSizeOnce = uHandler->bufferSize / uHandler->bufferBlocks;
    // inc write pointer
    // Each time an interrupt is triggered, pRXw must be an integer multiple of rxSizeOnce.
    uHandler->pRXw = (char *)(((int)uHandler->pRXw + rxSizeOnce) & ~(rxSizeOnce - 1));
    // record data count
    uHandler->usefulBuf = (uHandler->usefulBuf + rxSizeOnce) & ~(rxSizeOnce - 1);
    // check write pointer validity
    if(uHandler->pRXw > (uHandler->pucBuffer + uHandler->bufferSize))
        panic("Illegal pointer writing of the RX ring buffer from RX DMA Interrupt!!!");
    // address reset
    if(uHandler->pRXw == (uHandler->pucBuffer + uHandler->bufferSize))
        uHandler->pRXw = uHandler->pucBuffer;
    
    // is the ring buffer almost full???
    if((int)abs(uHandler->pRXr - uHandler->pRXw) < rxSizeOnce && uHandler->pRXr >= uHandler->pRXw)
    {
        // the ring buffer is almost full!!!

    }

    // Clear the interrupt request.
    if(DMA_IRQ_0 == uHandler->irq_num)
        dma_hw->ints0 |= 1u << uHandler->RxChannel;
    if(DMA_IRQ_1 == uHandler->irq_num)
        dma_hw->ints1 |= 1u << uHandler->RxChannel;
}