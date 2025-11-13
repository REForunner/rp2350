#pragma once

#ifndef LCD_H_
#define LCD_H_

#pragma pack (1) /* adjust to 1 byte */


/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <limits.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Example includes. */
#include "FreeRTOS_CLI.h"
#include "rp2350.h"

/*-----------------------------------------------------------*/

/* Task Parameter Analysis */
typedef struct lcd_t
{
    uint const pio;
    uint sm;
    uint const SidePinBase;
    uint const OutPin;
    uint const BackLightPin;
    float const div;
} lcd_t;

/*-----------------------------------------------------------*/

/// @brief lcd task
/// @param pv : pointer of lcd_t
void vLCDTask(void * pv);

/*-----------------------------------------------------------*/

#pragma pack () 

#endif /* LCD_H_ */