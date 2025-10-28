#include "cli.hpp"

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

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) )

/*
 * Implements the task-stats command.
 */
static BaseType_t prvTaskStatsCommand( char * pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char * pcCommandString )
{
    const char * const pcHeader = "Task\t\tState\tPrior\tStack\tNumber\tAffinity Mask\r\n";
    const char * const pcGap = "-------------------------------------------------------------\r\n";
    BaseType_t xSpacePadding;

    /* Remove compile time warnings about unused parameters, and check the
     * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     * write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Generate a table of task stats. */
    ( void ) strncpy( pcWriteBuffer, pcHeader, xWriteBufferLen );
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcGap, xWriteBufferLen );
    vTaskList( pcWriteBuffer + strlen( pcWriteBuffer ) );
    ( void ) strncpy( pcWriteBuffer + strlen( pcWriteBuffer ), pcGap, xWriteBufferLen );

    /* There is no more data to return after this single string, so return
     * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "task-stats" command line command.  This generates
 * a table that gives information on each task in the system. */
commandREGISTER static const CLI_Command_Definition_t xTaskStats =
{
    "task",                 /* The command string to type. */
    "\r\ntask:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
    prvTaskStatsCommand,    /* The function to run. */
    0                       /* No parameters are expected. */
};

#endif /* ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) ) */

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

    /* format string */
    int len = snprintf(pcWriteBuffer, xWriteBufferLen, "uid: 0x%llX\r\n", board_id);

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
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    ( void ) snprintf( pcWriteBuffer, xWriteBufferLen, "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", ( int ) xPortGetFreeHeapSize(), ( int ) xPortGetMinimumEverFreeHeapSize() );

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
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

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
    "\r\ntrace [start | stop]:\r\n Starts or stops a trace recording for viewing in FreeRTOS+Trace\r\n",
    prvStartStopTraceCommand, /* The function to run. */
    1                         /* One parameter is expected.  Valid values are "start" and "stop". */
};

#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */

/*-----------------------------------------------------------*/