#pragma once

#ifndef LCD_H_
#define LCD_H_

#pragma pack (1) /* adjust to 1 byte */


/* Standard includes. */
#include <string.h>
#include <stdio.h>
#include <limits.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Example includes. */
#include "FreeRTOS_CLI.h"
#include "rp2350.h"


/*-----------------------------------------------------------*/

/* define default scan direction */
#ifndef DFT_SCAN_DIR
    #define DFT_SCAN_DIR    D2U_L2R
#endif

/* st7789 Origin offset amount */
/* in Horizontal mode */
#define ST7789_OFFSET_X_W_Hor   40
#define ST7789_OFFSET_Y_H_Hor   52
/* in Vertical mode */
#define ST7789_OFFSET_X_W_Ver   52
#define ST7789_OFFSET_Y_H_Ver   40

/*-----------------------------------------------------------*/

/* display direction */
typedef enum displaydir_t
{
    Vertical,
    Horizontal,
} displaydir_t;

/* scan direction */
typedef enum scandir_t
{
    L2R_U2D = 0,           /* left to right, up to down → ↓ */
    L2R_D2U = 1,           /* left to right, down to up → ↑ */
    R2L_U2D = 2,           /* right to left, up to down ← ↓ */
    R2L_D2U = 3,           /* right to left, down to up ← ↑ */
    U2D_L2R = 4,           /* up to down, left to right ↓ → */
    U2D_R2L = 5,           /* up to down, right to left ↓ ← */
    D2U_L2R = 6,           /* down to up, left to right ↑ → */
    D2U_R2L = 7,           /* down to up, right to left ↑ ← */
} scandir_t;

/* Task Parameter Analysis */
typedef struct lcd_t
{
    // pio hardware information
    uint const pio;
    uint sm;
    int dmaChannel;
    uint const SidePinBase;
    uint const OutPin;
    uint const BackLightPin;
    float const div;
    // lcd information
    displaydir_t dir;
    uint16_t width;
    uint16_t height;
} lcd_t;

/*-----------------------------------------------------------*/

/// @brief lcd task
/// @param pv : pointer of lcd_t
void vLCDTask(void * pv);

/*-----------------------------------------------------------*/

#pragma pack () 

#endif /* LCD_H_ */