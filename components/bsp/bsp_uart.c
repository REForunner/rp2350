#include "bsp.h"
#include "bsp_uart.h"

// dma's irq by uart used
#define UART_DMA_IRQ_NUM    DMA_IRQ_1
// set dma channel'irq enabled/disabled (Must correspond to "UART_DMA_IRQ_NUM")
#define UART_DMA_CHANEEL_IRQ_SET(x, y)  dma_channel_set_irq1_enabled(x, y)
// uart buffer size max
#define UART_BUFFER_BYTES_MAX       32768U
// dma irq priority
#define UART_RX_DMA_PRIORITY        PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY
#define UART_TX_DMA_PRIORITY        PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY

/// @brief Initialise a UART with DMA(tx and rx).
/// @param tx : the GPIO number of tx.
/// @param rx : the GPIO number of rx.
/// @param px : Pointer to the configuration result structure.
void vbspUartInitWithDMA(uint tx, uint rx, UartHandler_t * px)
{
    // check param
    if(NULL == px->uart || NULL == px->TXdmaCb || NULL == px->RXdmaCb || NULL == px->uartCb)
        panic("Invalid UART configuration parameters!!!");
    if(NUM_BANK0_GPIOS <= tx || NUM_BANK0_GPIOS  <= rx )
        panic("Invalid GPIO number for UART!!!");

    px->irq_num = (uint)UART_DMA_IRQ_NUM;
    // Initialise a UART and return the Actual configured baud rate
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    px->txGPIO = tx;
    px->rxGPIO = rx;
    gpio_set_function(tx, UART_FUNCSEL_NUM(px->uart, tx));
    gpio_set_function(rx, UART_FUNCSEL_NUM(px->uart, rx));
    
    // Set up our UART with the required speed.

    /* 
        Default coniguration are following:
            FIFOs enable(default 1/2 full tigger)
            8 data bits
            No parity bit
            One stop bit
    */
    
    // return the Actual configured baud rate, and enable uart
    px->baudrate = uart_init(px->uart, px->baudrate);
    // if uart enabled, close it. because "uarrt_init" will open.
    if(uart_is_enabled(px->uart))
    {
        // disable uart
        uart_get_hw(px->uart)->cr ^= UART_UARTCR_UARTEN_BITS;
    }
    // open DMA is disabled when the uart error interrupt is asserted.
    // it must be done after the call "uart_init", Otherwise, it will overwrite.
    uart_get_hw(px->uart)->dmacr |= UART_UARTDMACR_DMAONERR_BITS;
    // interrupt enable of all error and RTIM(receive timeout)
    uart_get_hw(px->uart)->imsc = UART_UARTIMSC_RTIM_BITS | // receive time(line ilde)
                                UART_UARTIMSC_FEIM_BITS |   // framing error
                                UART_UARTIMSC_PEIM_BITS |   // parity error
                                UART_UARTIMSC_BEIM_BITS |   // line break(if rx always low, it will be tiggered!!!)
                                UART_UARTIMSC_OEIM_BITS;    // overrun error
    // Configure the processor to run call-back when UART_IRQ0 is asserted
    irq_set_exclusive_handler(UART_IRQ_NUM(px->uart), px->uartCb);
    
    // Set specified interrupt’s priority.
    // irq_set_priority(UART_IRQ_NUM(px->uart), UART_IRQ_PRIORITY);
    
    // enable uart0 interrupt on current core(executing this program core)
    // And other core isn't open this interrupt!!!
    irq_set_enabled(UART_IRQ_NUM(px->uart), true);

    // Get a free channel for tx, panic() if there are none
    px->TxChannel = dma_claim_unused_channel(true);
    // Configure dma channel for uart tx
    dma_channel_config TxC = dma_channel_get_default_config(px->TxChannel);
    channel_config_set_transfer_data_size(&TxC, DMA_SIZE_8);
    channel_config_set_dreq(&TxC, UART_DREQ_NUM(px->uart, true));
    channel_config_set_read_increment(&TxC, true);
    channel_config_set_write_increment(&TxC, false); 
    // config channel
    dma_channel_configure(
            px->TxChannel,              // Channel to be configured
            &TxC,                       // The configuration
            &uart_get_hw(px->uart)->dr, // The initial write address
            NULL,                       // read address(Configuration at sending)
            0,                          // Number of transfers(Configuration at sending)
            false                       // NO Start immediately.
        );
    // add tx dma irq serve
    irq_add_shared_handler(px->irq_num, px->TXdmaCb, UART_TX_DMA_PRIORITY);

    // Get a free channel for rx, panic() if there are none
    px->RxChannel = dma_claim_unused_channel(true);
    // Configure dma channel for uart rx
    dma_channel_config RxC = dma_channel_get_default_config(px->RxChannel);
    channel_config_set_transfer_data_size(&RxC, DMA_SIZE_8);
    channel_config_set_dreq(&RxC, UART_DREQ_NUM(px->uart, false));
    /*
        hardware receive ring buffer depend on DMA feature:
            set read address no increment.
            set write address increment.
            set dma ctrl register ring_sel is write(true).
            set dma ctrl register ring_size ( buffer_size = 1 << ring_size ).
            set channel trans_count register mode is 0x1(becase it will set off to interrupt.)
        
        Note:
            1. ring_size must be equal to buffer size
            2. number of transfers once represents the percentage of usage( the once is (100/UART_RXBUFF_BLOCK_NUM)% )
                (Only related to DMA writes!!!)
    */

    // check buffer size, it must be equal to 2^n(max 32KB)
    int n = lUtilCheckAndFindOneLocal(UART_RX_BUFFER_BYTES_ALL_BLOCK);
    if(n == -1 || UART_RX_BUFFER_BYTES_ALL_BLOCK > UART_BUFFER_BYTES_MAX || (UART_RX_BUFFER_BYTES_ALL_BLOCK % 2 != 0U))
    {
        panic("Invaild buffersize! it must be equal to 2^n. And the buffer size max is 32768 Bytes!!!");
    }
    channel_config_set_read_increment(&RxC, false);
    channel_config_set_write_increment(&RxC, true);    
    channel_config_set_ring(&RxC, true, (uint)n); // 1 << n byte boundary on write ptr
    // config channel
    dma_channel_configure(
            px->RxChannel,              // Channel to be configured
            &TxC,                       // The configuration
            px->pucBuffer,              // The initial write address
            &uart_get_hw(px->uart)->dr, // read address
            UART_RX_BUFFER_BYTES_ALL_BLOCK / UART_RXBUFF_BLOCK_NUM,   // Number of transfers once
            true                        // Start immediately.
        );
    // add rx dma irq serve
    irq_add_shared_handler(px->irq_num, px->RXdmaCb, UART_RX_DMA_PRIORITY);
    
    // Tell the DMA to raise IRQ line when the channel finishes a block
    UART_DMA_CHANEEL_IRQ_SET(px->TxChannel, true);
    UART_DMA_CHANEEL_IRQ_SET(px->RxChannel, true);

    // Set specified interrupt’s priority, and enable it
    // irq_set_priority(UART_IRQ_NUM(px->uart), UART_DMA_CHANNEL_IRQ_PRIORITY);
    
    // enable rx and tx channel irq
    irq_set_enabled(px->irq_num, true);
    // uart enable
    uart_get_hw(px->uart)->cr |= UART_UARTCR_UARTEN_BITS;
}