#include "lcd.hpp"


/*-----------------------------------------------------------*/

/* The LCD requires the initialization of a set of command/parameter values */
typedef struct
{
    uint8_t cmd;
    uint32_t data[4];
    uint8_t databytes;  /* There is no data in the data; Bit 7 = the set delay; 0xFF = end of cmds */
} lcd_init_cmd_t;

/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/

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
    uint8_t regval;
    uint16_t temp;
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
    // switch width and height
    if(regval & 0x20)
    {
        if (px->width < px->height)   /* 交换X,Y */
        {
            temp = px->width;
            px->width = px->height;
            px->height = temp;
        }
    }
    else
    {
        if (px->width > px->height)   /* 交换X,Y */
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
void vLCDSetWindow(lcd_t * px, uint16_t xstar, uint16_t xend, uint16_t ystar, uint16_t yend)
{
    uint32_t address_x;
    uint32_t address_y;
    /* Landscape Mode */
    if (px->dir == Horizontal)                  
    {
        uint8_t * databuf = (uint8_t *)&address_x;
        databuf[3] = (xstar + 40) >> 8;
        databuf[2] = 0xFF & (xstar + 40);
        databuf[1] = (xend + 40) >> 8;
        databuf[0] = 0xFF & (xend + 40);
        
        databuf = (uint8_t *)&address_y;
        databuf[3] = (ystar + 52) >> 8;
        databuf[2] = 0xFF & (ystar + 52);
        databuf[1] = (yend + 52) >> 8;
        databuf[0] = 0xFF & (yend + 52);
    }
    /* Portrait Mode */
    else
    {
        uint8_t * databuf = (uint8_t *)&address_x;
        databuf[3] = (xstar + 52) >> 8;
        databuf[2] = 0xFF & (xstar + 52);
        databuf[1] = (xend + 52) >> 8;
        databuf[0] = 0xFF & (xend + 52);
        
        databuf = (uint8_t *)&address_y;
        databuf[3] = (ystar + 40) >> 8;
        databuf[2] = 0xFF & (ystar + 40);
        databuf[1] = (yend + 40) >> 8;
        databuf[0] = 0xFF & (yend + 40);
    }
    // send x address
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x2A, &address_x, 4);
    // send y address
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x2B, &address_y, 4);
}

/// @brief 
/// @param px 
/// @param pul 
/// @param ulBytes 
void vLCDWriteRAM(lcd_t * px, uint32_t * pul, uint32_t ulBytes)
{
    vlcd_put_cmdAnddata(PIO_INSTANCE(px->pio), px->sm, 0x2C, pul, ulBytes);
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

    // init back-light
    prvLCDBackLightInit(BackLightPin);
    LCD_BL_PIN_OFF();
    // send lcd sequence
    prvLCDInitSequence(pio, sm);
    // config scan direction
    prvLCDSetScanDir(px, DFT_SCAN_DIR);

    vLCDSetWindow(px, 0, px->width, 0, px->height);

    uint32_t * pul = (uint32_t *)pvPortCalloc(1, SCREEN_TOTAL_BUFFER_SIZE);

    memset(pul, 0xff, SCREEN_TOTAL_BUFFER_SIZE);
    vLCDWriteRAM(px, pul, SCREEN_TOTAL_BUFFER_SIZE);

    uint32_t ulReceivedValue;
    
    while(true)
    {
        /* Block to wait for other Task() to notify this task. */  
        xTaskNotifyWait( 0, ULONG_MAX, &ulReceivedValue, pdMS_TO_TICKS(portMAX_DELAY) );
        /* give a vlue to pio sm */
        vlcd_put32(pio, sm, ulReceivedValue);
        /* wait pio sm idle */
        vlcd_wait_idle(pio, sm);
    }
}

/*-----------------------------------------------------------*/