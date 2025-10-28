/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * NOTE:  This file uses a third party USB CDC driver.
 */

#include "cli.hpp"

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE          512

/* DEL acts as a backspace. */
#define cmdASCII_DEL               ( 0x7F )

/*-----------------------------------------------------------*/

// std namespace statement of use
using namespace std;

/*-----------------------------------------------------------*/

/* The task that implements the command console processing. */
static void prvCommandConsoleTask( void * pvParameters );

/*-----------------------------------------------------------*/

/* Const messages output by the command console. */
static const char * const pcWelcomeMessage = "FreeRTOS command server.\r\nType Help to view a list of registered commands.\r\n\r\n>";
static const char * const pcEndOfOutputMessage = "\r\n[Press ENTER to execute the previous command again]\r\n\r\n>";
static const char * const pcNewLine = "\r\n\r\n";
static const char * const pcOverFlow = "The data is overflow. The current command will be ignored.\r\n\r\n>";

/*-----------------------------------------------------------*/

static void prvCommandConsoleTask( void * pvParameters )
{
    int lRxSize = 0;
    uint8_t ucInputIndex = 0;
    char * pcOutputString = NULL;
    BaseType_t xReturned = pdFALSE;
    /* It must be initialized; otherwise, the program will not function properly. */
    char cRxedChar[ cmdMAX_INPUT_SIZE ] = { 0 };
    char cInputString[ cmdMAX_INPUT_SIZE ] = { 0 }, cLastInputString[ cmdMAX_INPUT_SIZE ] = { 0 };
    
    cli_t * px = (cli_t *)pvParameters;

    /* Obtain the address of the output buffer.  Note there is no mutual
     * exclusion on this buffer as it is assumed only one command console interface
     * will be used at any one time. */
    pcOutputString = FreeRTOS_CLIGetOutputBuffer();

    /* Send the welcome message. */
    px->w((uint8_t *)pcWelcomeMessage, strlen(pcWelcomeMessage));

    for( ; ; )
    {
        /* Wait for the character. */
        lRxSize = px->r((uint8_t *)cRxedChar, (int)cmdMAX_INPUT_SIZE);
        /* check overflow */
        if(0U > lRxSize)
        {
            /* output note: data overflow */
            px->w((uint8_t *)pcOverFlow, strlen(pcOverFlow));
            continue;
        }
        /* cli start execution  */
        for(int i = 0; i < lRxSize; i++)
        {
            /* Was it the end of the line? */
            if( ( cRxedChar[i] == '\n' ) || ( cRxedChar[i] == '\r' ) )
            {
                /* Just to space the output from the input. */
                px->w((uint8_t *)pcNewLine, strlen(pcNewLine));

                /* See if the command is empty, indicating that the last command
                 * is to be executed again. */
                if( ucInputIndex == 0 )
                {
                    /* Copy the last command back into the input string. */
                    memcpy( cInputString, cLastInputString, cmdMAX_INPUT_SIZE );
                }

                /* Pass the received command to the command interpreter.  The
                 * command interpreter is called repeatedly until it returns
                 * pdFALSE	(indicating there is no more output) as it might
                 * generate more than one string. */
                do
                {
                    /* Get the next output string from the command interpreter. */
                    xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

                    /* Write the generated string to the data interaction. */
                    px->w((uint8_t *)pcOutputString, strlen(pcOutputString));

                } while( xReturned != pdFALSE );

                /* All the strings generated by the input command have been
                 * sent.  Clear the input string ready to receive the next command.
                 * Remember the command that was just processed first in case it is
                 * to be processed again. */
                memcpy( cLastInputString, cInputString, cmdMAX_INPUT_SIZE );
                ucInputIndex = 0;
                memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

                px->w((uint8_t *)pcEndOfOutputMessage, strlen(pcEndOfOutputMessage));

                /* wait next command */
                break;
            }
            else
            {
                if( cRxedChar[i] == '\r' )
                {
                    /* Ignore the character. */
                }
                else if( ( cRxedChar[i] == '\b' ) || ( cRxedChar[i] == cmdASCII_DEL ) )
                {
                    /* Backspace was pressed.  Erase the last character in the
                     * string - if any. */
                    if( ucInputIndex > 0 )
                    {
                        ucInputIndex--;
                        cInputString[ ucInputIndex ] = '\0';
                    }
                }
                else
                {
                    /* A character was entered.  Add it to the string entered so
                     * far.  When a \n is entered the complete	string will be
                     * passed to the command interpreter. */
                    if( ( cRxedChar[i] >= ' ' ) && ( cRxedChar[i] <= '~' ) )
                    {
                        if( ucInputIndex < cmdMAX_INPUT_SIZE )
                        {
                            cInputString[ ucInputIndex ] = cRxedChar[i];
                            ucInputIndex++;
                        }
                    }
                }
            }
        }
        
    }
}

/*-----------------------------------------------------------*/

/* Create that task that handles the console itself. */
/// @brief create cli task
/// @param pvParameters : read and write interaction
/// @param pxCreatedTask ： task handle
/// @return the resualt of Task creation
BaseType_t xCLIStart( void * const pvParameters, TaskHandle_t * const pxCreatedTask )
{
    // check param
    if(nullptr == pvParameters)
    {
        return pdFAIL;
    }

    /* Register all the command line commands defined immediately above. */
    extern void vCommandRegister(void);
    vCommandRegister();

    /* Create that task that handles the console itself. */
    return xTaskCreate( prvCommandConsoleTask,     /* The task that implements the command console. */
                        "cli",                     /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
                        3072U,                     /* The size of the stack allocated to the task. 3K * 4B = 12K */
                        pvParameters,
                        configTIMER_TASK_PRIORITY - 1,  /* Only one level lower in priority than timer task */
                        pxCreatedTask );
}

/*-----------------------------------------------------------*/