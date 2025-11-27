#include "cli.hpp"
#include <inttypes.h>

/*-----------------------------------------------------------*/

#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
    #define configINCLUDE_TRACE_RELATED_CLI_COMMANDS    0
#endif

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
    #define configINCLUDE_QUERY_HEAP_COMMAND    0
#endif

/*-----------------------------------------------------------*/

/* The location where Declaration CLI_Command_Definition_t is stored */
#define commandREGISTER __attribute__((used, section("xCLICommand")))

/*-----------------------------------------------------------*/

/* Linker-provided section boundaries for commands.
 * Declare common symbol names as weak and with C linkage so that:
 * - code compiles even if a particular symbol variant is not provided by
 *   the linker script (weak attribute), and
 * - the names are not mangled when compiled as C++ (extern "C").
 */
#ifdef __cplusplus
extern "C" {
#endif
extern CLI_Command_Definition_t __start_xCLICommand[] __attribute__((weak));
extern CLI_Command_Definition_t __stop_xCLICommand[] __attribute__((weak));
extern CLI_Command_Definition_t xCLICommand$$Base __attribute__((weak));
extern CLI_Command_Definition_t xCLICommand$$Limit __attribute__((weak));
#ifdef __cplusplus
}
#endif

/// @brief register all command
/// @param  None
void vCommandRegister(void)
{
    /* Determine the start/end of the command section. Prefer __start/__stop
     * if available; otherwise fall back to xCLICommand$$Base/Limit. Because
     * these are weak symbols, they may be NULL (not provided), so check
     * before using.
     */
    CLI_Command_Definition_t *start = nullptr;
    CLI_Command_Definition_t *end = nullptr;

    if ((void *)__start_xCLICommand != nullptr && (void *)__stop_xCLICommand != nullptr) 
    {
        start = __start_xCLICommand;
        end = __stop_xCLICommand;
    } 
    else if ((void *)&xCLICommand$$Base != nullptr && (void *)&xCLICommand$$Limit != nullptr) 
    {
        start = reinterpret_cast<CLI_Command_Definition_t *>(&xCLICommand$$Base);
        end = reinterpret_cast<CLI_Command_Definition_t *>(&xCLICommand$$Limit);
    }
    else 
    {
        /* No section symbols available — nothing to register. */
        return;
    }

    for (CLI_Command_Definition_t * px = start; px < end; ++px) {
        FreeRTOS_CLIRegisterCommand(px);
    }
}

/*-----------------------------------------------------------*/

#if ( ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) )

/*
 * Implements the task-stats command.
 */
static BaseType_t prvTaskStatsCommand( char * pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char * pcCommandString )
{
    const char * const pcHeader = "Task\t\tState\tPrior\tStack\tNumber\tAffinity-Mask\r\n";
    const char * const pcGap = "-------------------------------------------------------------\r\n";
    BaseType_t xSpacePadding;

    /* Remove compile time warnings about unused parameters, and check the
     * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     * write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );
    /* Generate a table of task stats. */
    ( void ) strncpy( pcWriteBuffer, pcHeader, xWriteBufferLen );
    /* Note: When used continuously, Pay attention to the remaining length!!!!! */
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcGap, xWriteBufferLen - strlen(pcWriteBuffer) );
    vTaskList( pcWriteBuffer + strlen( pcWriteBuffer ) );
    /* Note: When used continuously, Pay attention to the remaining length!!!!! */
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcGap, xWriteBufferLen - strlen(pcWriteBuffer) );

#if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) )
    
    const char * const pcTimeHeader = "Task\t\tCPU-Time\tUsage\r\n";
    const char * const pcTimeGap = "---------------------------------------\r\n";
    /* Start a new line */
    /* Note: When used continuously, Pay attention to the remaining length!!!!! */
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), "\r\n", xWriteBufferLen - strlen(pcWriteBuffer) );
    /* Generate a table of task cpu usage. */
    /* Note: When used continuously, Pay attention to the remaining length!!!!! */
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcTimeHeader, xWriteBufferLen - strlen(pcWriteBuffer) );
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcTimeGap, xWriteBufferLen - strlen(pcWriteBuffer) );
    /* get cpu run time information */
    vTaskGetRunTimeStats( pcWriteBuffer + strlen( pcWriteBuffer ) );
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcTimeGap, xWriteBufferLen - strlen(pcWriteBuffer) );    

#endif /* ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) ) */

    /* There is no more data to return after this single string, so return
     * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "task-stats" command line command.  This generates
 * a table that gives information on each task in the system. */
commandREGISTER static const CLI_Command_Definition_t xTaskStats =
{
    "task-stats",                 /* The command string to type. */
    "\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
    prvTaskStatsCommand,    /* The function to run. */
    0                       /* No parameters are expected. */
};

#endif /* ( ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) && ( configUSE_TRACE_FACILITY == 1 ) ) */

/*-----------------------------------------------------------*/

/*
 * Implements the get unique id command.
 */
static BaseType_t prvGetUID( char * pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const char * pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
     * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     * write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );
    /* format string */
    ( void ) snprintf(pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen -  strlen(pcWriteBuffer), "uid: 0x");
    /* uid */
    for(int i =0; i< sizeof(board_id); i++)
    {
        ( void ) snprintf(pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen -  strlen(pcWriteBuffer), "%X", board_id.id[i]);
    }
    /* new line */
    ( void ) snprintf(pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen -  strlen(pcWriteBuffer), "\r\n");

    /* There is no more data to return after this single string, so return
     * pdFALSE. */
    return pdFALSE;
}

/* get chip uid */
commandREGISTER static const CLI_Command_Definition_t xUID =
{
    "uid",          /* The command string to type. */
    "\r\nuid:\r\n Displays the unique id of the current chip\r\n",
    prvGetUID,      /* The function to run. */
    0               /* No parameters are expected. */
};

/*-----------------------------------------------------------*/

#if ( configINCLUDE_QUERY_HEAP_COMMAND == 1 )

/*
 * Implements the "query heap" command.
 */
static BaseType_t prvQueryHeapCommand( char * pcWriteBuffer,
                                        size_t xWriteBufferLen,
                                        const char * pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
        * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        * write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );
    /* print the heap numbers */
    ( void ) snprintf( pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen - strlen(pcWriteBuffer), "Having two heap areas:\r\n\r\n" );
    /* print one of the heap information */
    /* Iterate until the sentinel { NULL, 0 } entry in xHeapRegions is found. */
    for (int i = 0; xHeapRegions[i].pucStartAddress != NULL; ++i)
    {
        ( void ) snprintf( pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen - strlen(pcWriteBuffer), "\t[%d]: Address: 0x%" PRIXPTR "\tSize(bytes): 0x%zX\r\n\r\n", i, (uintptr_t)xHeapRegions[i].pucStartAddress, xHeapRegions[i].xSizeInBytes );
    }
    /* print the heap statsus */
    ( void ) snprintf( pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen - strlen(pcWriteBuffer), "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", ( int ) xPortGetFreeHeapSize(), ( int ) xPortGetMinimumEverFreeHeapSize() );

    /* There is no more data to return after this single string, so return
        * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "query_heap" command line command. */
commandREGISTER static const CLI_Command_Definition_t xQueryHeap =
{
    "query-heap",
    "\r\nquery-heap:\r\n Displays the free heap space, and minimum ever free heap space.\r\n",
    prvQueryHeapCommand, /* The function to run. */
    0                    /* The user can enter any number of commands. */
};

#endif /* configINCLUDE_QUERY_HEAP */

/*-----------------------------------------------------------*/

#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1

/*
 * Implements the "trace start" and "trace stop" commands;
 */
static BaseType_t prvStartStopTraceCommand( char * pcWriteBuffer,
                                            size_t xWriteBufferLen,
                                            const char * pcCommandString )
{
    const char * pcParameter;
    BaseType_t lParameterStringLength;

    /* Remove compile time warnings about unused parameters, and check the
        * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        * write buffer length is adequate, so does not check for buffer overflows. */
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );
    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter
                    (
                        pcCommandString,        /* The command string itself. */
                        1,                      /* Return the first parameter. */
                        &lParameterStringLength /* Store the parameter string length. */
                    );

    /* Sanity check something was returned. */
    configASSERT( pcParameter );

    /* There are several valid parameter values: start, stop, status. */
    if( strncmp( pcParameter, "start", strlen( "start" ) ) == 0 )
    {
        /* Start or restart the trace. Use safe formatted output. */
        /* Stop/clear first to ensure a fresh recording. */
        // vTraceStop();
        // vTraceClear();
        // vTraceStart();
     
        ( void ) snprintf( pcWriteBuffer, xWriteBufferLen, "Trace recording (re)started.\r\n" );
    }
    else if( strncmp( pcParameter, "stop", strlen( "stop" ) ) == 0 )
    {
        /* End the trace, if one is running. */
        // vTraceStop();
        
        ( void ) snprintf( pcWriteBuffer, xWriteBufferLen, "Stopping trace recording.\r\n" );
    }
    else if( strncmp( pcParameter, "status", strlen( "status" ) ) == 0 )
    {
        /* Report a best-effort status. FreeRTOS+Trace does not expose a
         * standard API to query running state in all ports, so we report a
         * generic message. If the trace library in this build exposes a
         * status API, replace the following with that call.
         */
        ( void ) snprintf( pcWriteBuffer, xWriteBufferLen, "Trace status: unknown (recording state not queryable). Use 'start' or 'stop'.\r\n" );
    }
    else
    {
        ( void ) snprintf( pcWriteBuffer, xWriteBufferLen, "Valid parameters are 'start', 'stop' and 'status'.\r\n" );
    }

    /* There is no more data to return after this single string, so return
        * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "trace" command line command.  This takes a single
 * parameter, which can be either "start" or "stop". */
commandREGISTER static const CLI_Command_Definition_t xStartStopTrace =
{
    "trace",
    "\r\ntrace <start | stop>:\r\n Starts or stops a trace recording for viewing in FreeRTOS+Trace\r\n",
    prvStartStopTraceCommand, /* The function to run. */
    1                         /* One parameter is expected.  Valid values are "start" and "stop". */
};

#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */

/*-----------------------------------------------------------*/

/*
 * Implements the "read" commands;
 */
static BaseType_t prvReadCommand( char * pcWriteBuffer,
                                  size_t xWriteBufferLen,
                                  const char * pcCommandString )
{
    const char * pcParameter;
    BaseType_t lParameterStringLength;
    char * ptr;

    static UBaseType_t ulAddressStart = 0U;
    static UBaseType_t ulReadSize = 0U;         // bytes
    static UBaseType_t ulHasReadSize = 0U;
    /* 
     * The loop for controlling the output content.
     *      bit[31] = 1: The parameters have been extracted.
     *      bit[30] = 1: The header has been printed.
     * 
     *      Clean up after each print job is completed.
     */
    static BaseType_t lFlag = 0U;

    const char * const pcHeader = "  Address\t00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F           text\r\n";
    const char * const pcGap = "------------------------------------------------------------------------------------\r\n";
    
    /* Remove compile time warnings about unused parameters, and check the
        * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        * write buffer length is adequate, so does not check for buffer overflows. */
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );
    
    /* Obtain the parameter string. */
    if(!(lFlag & (1UL << 31)))
    {
        /* Extract all parameters at once */
        for(size_t n = 1; n <= 2U; n += 1U)
        {
            /* Obtain them one by one, totaling two parameters */
            pcParameter = FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,        /* The command string itself. */
                                (UBaseType_t)n,         /* Return the first parameter. */
                                &lParameterStringLength /* Store the parameter string length. */
                            );
            /* Sanity check something was returned. */
            configASSERT( pcParameter );
            
            /**
             * Distinguish the base number system.
             *      binary : 2          --> 0b/0B
             *      octal : 8           --> 0
             *      decimal : 10        --> other
             *      hexadecimal : 16    --> 0x/0X
            */
            int ucNumberBase = (int)eUtilGetNumberBase(pcParameter);
            if((int)BASE_INVALID == ucNumberBase)
            {
                /* invalid parameter value */
                /* read complete and clear all variable */
                ulAddressStart = 0U;
                ulReadSize = 0U;         // bytes
                ulHasReadSize = 0U;
                lFlag = 0U;

                /* Note: When used continuously, Pay attention to the remaining length!!!!! */
                ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), "'read' : Incorrect command parameter(s)!!!\r\n", xWriteBufferLen - strlen(pcWriteBuffer) );

                /* There is no more data to return after this single string, so return
                    * pdFALSE. */
                return pdFALSE;
            }
            
            /* Convert a numeric string to a number */
            if(1U == n)
            {
                /* Extract the address */
                ulAddressStart = (UBaseType_t)strtoul(pcParameter, &ptr, ucNumberBase);
            }
            else
            {
                /* Extract the read size */
                ulReadSize = (UBaseType_t)strtoul(pcParameter, &ptr, ucNumberBase);
            }
        }

        /* The parameters have been extracted. */
        lFlag |= (1UL << 31);
    }

    /* creat and print header */
    if(!(lFlag & (1UL << 30)))
    {
        /* Generate a table of task stats. */
        ( void ) strncpy( pcWriteBuffer, pcHeader, xWriteBufferLen );
        /* Note: When used continuously, Pay attention to the remaining length!!!!! */
        ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcGap, xWriteBufferLen - strlen(pcWriteBuffer) );
        /* The header has been printed. */
        lFlag |= (1UL << 30);
    }

    /* Process the data in 16-byte blocks. */
    if(ulHasReadSize < ulReadSize) 
    {
        /* Printed Address (8-bit Hexadecimal) */
        ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "0x%08X\t", (unsigned int)ulAddressStart);
        
        /* Print out a 16-byte hexadecimal value */
        for (size_t j = 0; j < 16U; j++) 
        {
            /* When the data size is less than 16 bytes, fill with spaces */
            if (ulHasReadSize + j < ulReadSize)
            {
                ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "%02X ", *(uint8_t *)(ulAddressStart + j));
            }
            else
            {
                ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "   ");
            }
        }

        /* Print four space */
        ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "    ");

        /* Print the corresponding ASCII text */
        for (size_t m = 0; m < 16U; m++) 
        {
            if (ulHasReadSize + m < ulReadSize)
            {
                /* Printable characters are displayed directly, while non-printable characters are shown as '.' */
                unsigned char c = (unsigned char) *(uint8_t *)(ulAddressStart + m);
                ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "%c", isprint(c) ? c : '.');
            }
        }

        /* Print A new line */
        ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "\r\n");
        /* recard has read size and step address */
        ulHasReadSize += 16U;
        ulAddressStart += 0x10UL;

        /* There is more data to be returned as no parameters have been echoed
         * back yet. */
        return pdPASS;
    }
    else
    {
        /* read complete and clear all variable */
        ulAddressStart = 0U;
        ulReadSize = 0U;         // bytes
        ulHasReadSize = 0U;
        lFlag = 0U;

        /* Note: When used continuously, Pay attention to the remaining length!!!!! */
        ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcGap, xWriteBufferLen - strlen(pcWriteBuffer) );

        /* There is no more data to return after this single string, so return
         * pdFALSE. */
        return pdFALSE;
    }
}

/* Structure that defines the "read" command line command. */
commandREGISTER static const CLI_Command_Definition_t xRead =
{
    "r",
    "\r\nr <address> <number>:\r\n Read <number> bytes value from <address> to <address + number>.\r\n",
    prvReadCommand,     /* The function to run. */
    2                   /* The user can enter any number of commands. */
};

/*-----------------------------------------------------------*/

/*
 * Implements the lcd command.
 */
static BaseType_t prvLCD( char * pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const char * pcCommandString )
{
    const char * pcParameter;
    BaseType_t lParameterStringLength;
    char * ptr;

    /* Remove compile time warnings about unused parameters, and check the
        * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        * write buffer length is adequate, so does not check for buffer overflows. */
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );

    /* Obtain them one by one, totaling two parameters */
    pcParameter = FreeRTOS_CLIGetParameter
                    (
                        pcCommandString,        /* The command string itself. */
                        (UBaseType_t)1,         /* Return the first parameter. */
                        &lParameterStringLength /* Store the parameter string length. */
                    );
    /* Sanity check something was returned. */
    configASSERT( pcParameter );
    
    /**
     * Distinguish the base number system.
     *      binary : 2          --> 0b/0B
     *      octal : 8           --> 0
     *      decimal : 10        --> other
     *      hexadecimal : 16    --> 0x/0X
    */
    int ucNumberBase = (int)eUtilGetNumberBase(pcParameter);

    /* Extract the address */
    UBaseType_t ulAddressStart = (UBaseType_t)strtoul(pcParameter, &ptr, ucNumberBase);
    ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "0x%x", ulAddressStart);
    BaseType_t xResult = xTaskNotify( xLCDHandle, ulAddressStart, eSetValueWithOverwrite );

    /* There is no more data to return after this single string, so return
     * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "lcd" command line command. */
commandREGISTER static const CLI_Command_Definition_t xLCD =
{
    "lcd",
    "\r\nlcd <value>:\r\n .\r\n",
    prvLCD,     /* The function to run. */
    1                   /* The user can enter any number of commands. */
};

/*-----------------------------------------------------------*/

/*
 * Implements the clock command.
 */
static BaseType_t prvClock( char * pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const char * pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
        * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        * write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    configASSERT( pcWriteBuffer );

    /* clear write buffer */
    memset( pcWriteBuffer, 0x00, xWriteBufferLen );

    ( void ) snprintf(pcWriteBuffer + strlen( pcWriteBuffer ), xWriteBufferLen - strlen(pcWriteBuffer), "%d MHz\r\n", (clock_get_hz(clk_sys)));

    /* There is no more data to return after this single string, so return
     * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "lcd" command line command. */
commandREGISTER static const CLI_Command_Definition_t xClock =
{
    "clock",
    "\r\nclock:\r\n Displays the cpu frequency of the current.\r\n",
    prvClock,     /* The function to run. */
    0                   /* The user can enter any number of commands. */
};

/*-----------------------------------------------------------*/