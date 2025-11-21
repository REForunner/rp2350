#include "lcd.hpp"


/*-----------------------------------------------------------*/

/* The LCD requires the initialization of a set of command/parameter values */
typedef struct
{
    uint8_t cmd;
    uint32_t data[4];
    uint8_t databytes;  /* There is no data in the data; Bit 7 = the set delay; 0xFF = end of cmds */
} lcd_init_cmd_t;

/* lcd task ipc format */
/**
 * each two 16-bit pixel is a 32-bit word, that transmission format
 * typedef struct
 * {                       // msb
 *     uint16_t pixel_1;   // Pixel 1 in the lower half of the character
 *     uint16_t pixel_0;   // Pixel 0 in the upper half of the character
 * } __attribute__((packed)) pixel_t;
 */
typedef struct lcd_ipc_format_t
{
    // transfer by dma
    uint32_t data[SCREEN_TOTAL_BUFFER_SIZE / 4U];
    uint32_t dlen;
    // Coordinate setting used
    uint16_t xstar;
    uint16_t xend;
    uint16_t ystar;
    uint16_t yend;
} lcd_ipc_format_t;

/*-----------------------------------------------------------*/

// lcd cmd semaphore
static SemaphoreHandle_t xLCDDMABinary = NULL;
// m2m dma channel
static uint lcd_dma;

/*-----------------------------------------------------------*/

/// @brief lcd dma Interrupt service
/// @param void 
static void prvLCDDMAServe(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    // Is it time for vATask () to run?
    xHigherPriorityTaskWoken = pdFALSE;
    // tigger source
    if(!(dma_channel_get_irq0_status(lcd_dma)))
        return;

    // Clear the interrupt request.
    dma_hw->ints0 |= 1u << lcd_dma;
    // Unblock the task by releasing the semaphore.
    xSemaphoreGiveFromISR( xLCDDMABinary, &xHigherPriorityTaskWoken );
    // switch task??? if need.
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        // We can force a context switch here.  Context switching from an
        // ISR uses port specific syntax.  Check the demo task for your port
        // to find the syntax required.
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/// @brief dma init when lcd used
/// @param px : lcd_t handle
/// @return used channel
static int prvLCDDMAInit(lcd_t * px)
{
     /* creat a binary type semaphore */
    xLCDDMABinary = xSemaphoreCreateBinary();

    if( xLCDDMABinary == NULL )
    {
        /* The semaphore was created failed */
        panic("LCD DMA binary semaphore was created failed!!!");
    }

    // Get a free channel, panic() if there are none
    lcd_dma = dma_claim_unused_channel(true);
    // 8 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    dma_channel_config c = dma_channel_get_default_config(lcd_dma);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(PIO_INSTANCE(px->pio), px->sm, true));
    // channel_config_set_bswap(&c, true);
    dma_channel_configure(lcd_dma, &c, (volatile void *)&PIO_INSTANCE(px->pio)->txf[px->sm], NULL, 0, false);
    // Tell the DMA to raise IRQ line when the channel finishes a block
    dma_channel_set_irq0_enabled(lcd_dma, true);
    // add tx dma irq serve
    irq_add_shared_handler(DMA_IRQ_0, prvLCDDMAServe, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    // enable tx channel irq
    irq_set_enabled(DMA_IRQ_0, true);
    // return use channel
    return lcd_dma;
}

/// @brief send lcd init sequence
/// @param pio : PIO instance
/// @param sm : state machine number
static void prvLCDInitSequence(PIO const pio, uint const sm)
{
    // LCD initialization sequence
    // The data is in big-endian format.
    static const lcd_init_cmd_t xCmds_msb[] =
    {
        { 0x11, { 0 }, 0x80 },
        { 0x36, { 0x70 }, 1 },
        { 0x3A, { 0x05 }, 1 },
        { 0xB2, { 0x0C0C0033, 0x33 }, 5 },
        { 0xB7, { 0x35 }, 1 },
        { 0xBB, { 0x19 }, 1 },
        { 0xC0, { 0x2C }, 1 },
        { 0xC2, { 0x01 }, 1 },
        { 0xC3, { 0x12 }, 1 },
        { 0xC4, { 0x20 }, 1 },
        { 0xC6, { 0x01 }, 1 },
        { 0xD0, { 0xA4A1 }, 2 },
        { 0xE0, { 0xD0040D11, 0x132B3F54, 0x4C180D0B, 0x1F23 }, 14 },
        { 0xE1, { 0xD0040C11, 0x132C3F44, 0x512F1F1F, 0x2023 }, 14 },
        { 0x21, { 0 }, 0x80 },
        { 0x29, { 0 }, 0x80 },
        { 0x00, { 0 }, 0xff},
    };

    int cmd = 0;
    // send  init sequence
    while (xCmds_msb[cmd].databytes != 0xff)
    {
        // send data, msb first
        vlcd_put_cmdAnddata(pio, sm, xCmds_msb[cmd].cmd, (uint32_t *)xCmds_msb[cmd].data, xCmds_msb[cmd].databytes & 0x7F);
        // delay time
        if (xCmds_msb[cmd].databytes & 0x80)
        {
            vTaskDelay(pdMS_TO_TICKS(120));
        }
        // step-by-step
        cmd++;
    }
}

/// @brief init back-light control pin
/// @param gpio : gpio number of back-light control
static void prvLCDBackLightInit(uint gpio)
{
    // initialise back-light gpio
    gpio_init((uint)gpio);
    gpio_pull_down((uint)gpio);
    gpio_set_dir((uint)gpio, GPIO_OUT);
}

/// @brief set lcd default width and height based on direction
/// @param px : lcd_t handle
/// @param eDir : display direction
static void prvLCDSetDefaultHeighWitdhBaseDir(lcd_t * px, displaydir_t eDir)
{
    /* update display direction */
    px->dir = eDir;
    /* update parameter */
    switch(eDir)
    {
        case Vertical:
        {
            px->width = SCREEN_HEIGHT;
            px->height = SCREEN_WIDTH;
        }
        break;
        case Horizontal:
        {
            px->width = SCREEN_WIDTH;
            px->height = SCREEN_HEIGHT;
        }
        break;
        default:
        {
            panic("The display direction is non-supported!!!\r\n");
        }
        break;
    }
}

/// @brief config scan direction
/// @param px : lcd_t handle
/// @param eScan : scan direction
static void prvLCDSetScanDir(lcd_t * px, scandir_t eScan)
{
    uint8_t regval = 0;
    uint16_t temp = 0;
    // Based on the scanning direction, obtain the command parameters
    switch (eScan)
    {
        case L2R_U2D:
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;
        case L2R_D2U:
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;
        case R2L_U2D:
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;
        case R2L_D2U:
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;
        case U2D_L2R:
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;
        case U2D_R2L:
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;
        case D2U_L2R:
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;
        case D2U_R2L:
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
        default:
            panic("The scan direction is wrong!!!");
            break;
    }
    // send data, msb first
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x36, (uint32_t *)&regval, 1U);
    // // switch width and height
    if(regval & 0x20)
    {
        if (px->width < px->height)   /* switch X, Y */
        {
            temp = px->width;
            px->width = px->height;
            px->height = temp;
        }
    }
    else
    {
        if (px->width > px->height)   /* switch X, Y */
        {
            temp = px->width;
            px->width = px->height;
            px->height = temp;
        }
    }
}

/// @brief set a window
/// @param px : lcd_t handle
/// @param xstar : Top left x-axis
/// @param xend : The bottom right x-axis
/// @param ystar : Top left y-axis
/// @param yend : The bottom right y-axis
static void prvLCDSetWindow(lcd_t * px, uint16_t xstar, uint16_t xend, uint16_t ystar, uint16_t yend)
{
    uint32_t address_x;
    uint32_t address_y;
    /* in Horizontal mode */
    if(px->dir == Horizontal)
    {
        uint16_t * databuf = (uint16_t *)&address_x;
        /* Calculate the starting address of x */
        databuf[1] = xstar + (uint16_t)ST7789_OFFSET_X_W_Hor;
        databuf[0] = xend + (uint16_t)ST7789_OFFSET_X_W_Hor;
        /* Calculate the starting address of y */
        databuf = (uint16_t *)&address_y;
        databuf[1] = ystar + (uint16_t)ST7789_OFFSET_Y_H_Hor;
        databuf[0] = yend + (uint16_t)ST7789_OFFSET_Y_H_Hor;
    }
    /* in Vertical mode */
    else
    {
        uint16_t * databuf = (uint16_t *)&address_x;
        /* Calculate the starting address of x */
        databuf[1] = xstar + (uint16_t)ST7789_OFFSET_X_W_Ver;
        databuf[0] = xend + (uint16_t)ST7789_OFFSET_X_W_Ver;
        /* Calculate the starting address of y */
        databuf = (uint16_t *)&address_y;
        databuf[1] = ystar + (uint16_t)ST7789_OFFSET_Y_H_Ver;
        databuf[0] = yend + (uint16_t)ST7789_OFFSET_Y_H_Ver;
    }
    
    // send x address
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x2A, &address_x, 4);
    // send y address
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x2B, &address_y, 4);
}

/// @brief write st7789 gram
/// @param px : lcd_t handle
/// @param pul : written data pointer
/// @param ulBytes : data bytes
static void prvLCDWriteRAM(lcd_t * px, const uint32_t * pul, uint32_t ulBytes)
{
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x2C, pul, ulBytes);
}

/// @brief write st7789 use dma
/// @param px : lcd_t handle
/// @param cmd : lcd command
/// @param pul : written data pointer
/// @param ulBytes : data bytes
static void prvLCDWriteUseDMA(lcd_t * px, const uint8_t cmd, const uint32_t * pul, uint32_t ulBytes)
{
    /* 1. set cmd bits and cmd */
    vlcd_put32(PIO_INSTANCE(px->pio), px->sm, (0x00000700 | (uint32_t)cmd));
    /* 2. set send data all bits */
    if(0 == ulBytes || NULL == pul)
    {
        /* no cmd param(data), sm return start */
        vlcd_sm_return_when32DataBit(PIO_INSTANCE(px->pio), px->sm);
        /* exit */
        return;
    }
    else
    {
        /* have param(data) */
        vlcd_put32(PIO_INSTANCE(px->pio), px->sm, ((ulBytes * 8) - 1));
    }
    /* 3. send data */
    const uint8_t * puc = (const uint8_t *)pul;
    int count = ulBytes / sizeof(uint32_t);
    int remain = ulBytes % sizeof(uint32_t);
    /* 32-bit send */
    if(0 != count)
    {
        /* put param(data) */
        dma_channel_transfer_from_buffer_now(px->dmaChannel, (volatile void *)puc, count);
        /* wait dma finish */
        // dma_channel_wait_for_finish_blocking(px->dmaChannel);
        xSemaphoreTake( xLCDDMABinary, pdMS_TO_TICKS(portMAX_DELAY) );
        /* step-by-step */
        puc += (count * sizeof(uint32_t));
    }
    /* based on whether 32-bit alignment processing is performed */
    /* non-aligned on 32-bit */
    if(0 != remain)
    {
        /* 4. handling the remaining bytes */
        /* 4.1 16-bit format first */
        if(remain >= 2)
        {
            /* put remaining 16-bit */
            vlcd_put16(PIO_INSTANCE(px->pio), px->sm, *(uint16_t *)puc);  // ?????
            /* remaining bytes */
            remain -= 2;
            puc += 2;
        }
        /* 4.2 the last byte */
        if(0 != remain)
        {
            /* put remaining 8-bit */
            vlcd_put8(PIO_INSTANCE(px->pio), px->sm, puc[0]);  // ?????
        }
        /* non-aligned autoregression */
    }
    /* aligned on 32-bit */
    else
    {
        /* 5. determine whether manual regression is required */
        /* sm return start */
        vlcd_sm_return_when32DataBit(PIO_INSTANCE(px->pio), px->sm);
        /* exit */
        // return;
    }
}

/// @brief write st7789 gram use dma
/// @param px : lcd_t handle
/// @param pul : written data pointer
/// @param ulBytes : data bytes
static void prvLCDWriteGRAMUseDMA(lcd_t * px, const uint32_t * pul, uint32_t ulBytes)
{
    prvLCDWriteUseDMA(px, 0x2C, pul, ulBytes);
}

/// @brief write st7789 write memory continue use dma
/// @param px : lcd_t handle
/// @param pul : written data pointer
/// @param ulBytes : data bytes
static void prvLCDWriteContinueUseDMA(lcd_t * px, const uint32_t * pul, uint32_t ulBytes)
{
    prvLCDWriteUseDMA(px, 0x3C, pul, ulBytes);
}

/*-----------------------------------------------------------*/

/// @brief lcd task
/// @param pv : pointer of lcd_t
void vLCDTask(void * pv)
{
    lcd_t * px = (lcd_t *)pv;
    PIO const pio = PIO_INSTANCE(px->pio);
    uint const SidePinBase = (uint)px->SidePinBase;
    uint const OutPin = (uint)px->OutPin;
    uint const BackLightPin = (uint)px->BackLightPin;
    float div = (float)px->div;    

    // init pio driver to st7789
    uint sm = ullcd_program_init(pio, SidePinBase, OutPin, div);
    // must be init lcd finished
    px->sm = sm;
    px->dmaChannel = prvLCDDMAInit(px);

    // init back-light
    prvLCDBackLightInit(BackLightPin);
    LCD_BL_PIN_OFF();
    // send lcd sequence
    prvLCDInitSequence(pio, sm);
    // config scan direction
    prvLCDSetScanDir(px, DFT_SCAN_DIR);

    lcd_ipc_format_t * pul = (lcd_ipc_format_t *)pvPortCalloc(1, sizeof(lcd_ipc_format_t));
    LCD_BL_PIN_OFF();

    uint32_t ulReceivedValue;
    
    while(true)
    {
        /* Block to wait for other Task() to notify this task. */  
        xTaskNotifyWait( 0, ULONG_MAX, &ulReceivedValue, pdMS_TO_TICKS(portMAX_DELAY) );
        prvLCDSetScanDir(px, (scandir_t)ulReceivedValue);

        xTaskNotifyWait( 0, ULONG_MAX, &ulReceivedValue, pdMS_TO_TICKS(portMAX_DELAY) );
        uint16_t wx = ((uint16_t *)&ulReceivedValue)[0];
        uint16_t hy = ((uint16_t *)&ulReceivedValue)[1];
        prvLCDSetWindow(px, 0, wx - 1, 0, hy - 1);
        
        xTaskNotifyWait( 0, ULONG_MAX, &ulReceivedValue, pdMS_TO_TICKS(portMAX_DELAY) );
        /* give a vlue to pio sm */
        /* set buffer */
        for(int i = 0; i < wx * hy / 2; i++)
        {
            pul->data[i] = ulReceivedValue + i;
        }
       
        prvLCDWriteGRAMUseDMA(px, (const uint32_t *)pul->data, wx * hy * 2);
        
        /* wait pio sm idle */
        vlcd_wait_idle(pio, sm);
    }
}

/*-----------------------------------------------------------*/