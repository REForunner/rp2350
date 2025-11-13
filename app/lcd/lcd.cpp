#include "lcd.hpp"


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

    // initialise back-light gpio
    gpio_init((uint)BackLightPin);
    gpio_pull_down((uint)BackLightPin);
    gpio_set_dir((uint)BackLightPin, GPIO_OUT);
    LCD_BL_PIN_OFF();

    uint sm = ullcd_program_init(pio, SidePinBase, OutPin, div);
    // must be init lcd finished
    sm = px->sm;

    uint32_t * puc = (uint32_t *)pvPortCalloc(1, SCREEN_TOTAL_BUFFER_SIZE + 0x20);
    if(puc == NULL)
    {
        panic("lcd failed!!!");
    }

    vlcd_put_cmdAnddata(pio, sm, 0x01, NULL, 0);
    sleep_ms(100);
    vlcd_put_cmdAnddata(pio, sm, 0x11, NULL, 0);
    sleep_ms(100);
    puc[0] = 0x00000055;
    vlcd_put_cmdAnddata(pio, sm, 0x3a, puc, 1);
    sleep_ms(100);
    puc[0] = 0x00000000;
    vlcd_put_cmdAnddata(pio, sm, 0x36, puc, 1);
    sleep_ms(100);
    vlcd_put_cmdAnddata(pio, sm, 0x21, NULL, 0);
    sleep_ms(500);
    vlcd_put_cmdAnddata(pio, sm, 0x13, NULL, 0);
    sleep_ms(100);
    vlcd_put_cmdAnddata(pio, sm, 0x29, NULL, 0);
    sleep_ms(100);
    puc[0] = 0x00000086;
    vlcd_put_cmdAnddata(pio, sm, 0x2a, puc, 4);
    puc[0] = 0x000000ef;
    vlcd_put_cmdAnddata(pio, sm, 0x2b, puc, 4);
    memset(puc, 0xFF, SCREEN_TOTAL_BUFFER_SIZE);
    vlcd_put_cmdAnddata(pio, sm, 0x2c, puc, SCREEN_TOTAL_BUFFER_SIZE);

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