#pragma once

#ifndef CLI_H_
#define CLI_H_

/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Example includes. */
#include "FreeRTOS_CLI.h"
#include "rp2350.h"

/*-----------------------------------------------------------*/

/* command task name */
#define CLI_TASK_NAME           "cli"
/* command task stack size(32-bit word) */
#define CLI_TASK_STACK_SIZE     3072U

/*-----------------------------------------------------------*/

/* Prototype of data interaction function */
typedef int (* DataInteraction_t)(uint8_t *, int);

/* read and write interaction */
typedef struct cli_t
{
    DataInteraction_t r;    // read, if return -1, it represents overflow.
    DataInteraction_t w;    // write
} cli_t;


/*-----------------------------------------------------------*/

/* Create that task that handles the console itself. */
/// @brief create cli task
/// @param pvParameters : read and write interaction
/// @param pxCreatedTask ： task handle
/// @param uxPriority ： task priority
/// @return the resualt of Task creation
BaseType_t xCLIStart( void * const pvParameters, TaskHandle_t * const pxCreatedTask, UBaseType_t uxPriority );


/*-----------------------------------------------------------*/

#endif /* CLI_H_ */