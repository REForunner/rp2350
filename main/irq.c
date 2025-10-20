#include "irq.h"

/// @brief uart Interrupt service
/// @param void 
void virqUartServe(void)
{
    //
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
    

    // Clear the interrupt request.
    if(DMA_IRQ_0 == uHandler->irq_num)
        dma_hw->ints0 = 1u << uHandler->TxChannel;
    if(DMA_IRQ_1 == uHandler->irq_num)
        dma_hw->ints1 = 1u << uHandler->TxChannel;
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
    if(dma_channel_hw_addr(uHandler->RxChannel)->ctrl_trig & 0x80000000U)
        panic("Read or Write error has occured from UART RX DMA Channel!!!");
    
    // rx dma receives 1/UART_RXBUFF_BLOCK_NUM of buffer size datas.
    // no deal with!!!

    // Clear the interrupt request.
    if(DMA_IRQ_0 == uHandler->irq_num)
        dma_hw->ints0 = 1u << uHandler->RxChannel;
    if(DMA_IRQ_1 == uHandler->irq_num)
        dma_hw->ints1 = 1u << uHandler->RxChannel;
}