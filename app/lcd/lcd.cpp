#include "lcd.hpp"


/*-----------------------------------------------------------*/

/// @brief lcd task
/// @param pv : pointer of lcd_t
void vLCDTask(void * pv)
{
    lcd_t * px = (lcd_t *)pv;
    PIO pio = PIO_INSTANCE(px->pio);
    uint SidePinBase = (uint)px->SidePinBase;
    uint OutPin = (uint)px->OutPin;
    float div = (float)px->div;

    // Initialise pio
    uint sm = ullcd_program_init(pio, SidePinBase, OutPin, div);
    
    uint32_t ulReceivedValue;
    
    while(true)
    {
        /* Block to wait for other Task() to notify this task. */  
        xTaskNotifyWait( 0, ULONG_MAX, &ulReceivedValue, pdMS_TO_TICKS(portMAX_DELAY) );
        /* give a vlue to pio sm */
        vlcd_put(pio, sm, ulReceivedValue);
        /* wait pio sm idle */
        vlcd_wait_idle(pio, sm);
    }
}

/*-----------------------------------------------------------*/