#include "cli.hpp"

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
/// @param  NUll
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

static BaseType_t prvTaskStatsCommand( char * pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char * pcCommandString )
{
    const char * const pcHeader = "     State   Priority  Stack    #\r\n************************************************\r\n";
    BaseType_t xSpacePadding;

    /* Remove compile time warnings about unused parameters, and check the
     * write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     * write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Generate a table of task stats. */
    strcpy( pcWriteBuffer, "Task" );
    pcWriteBuffer += strlen( pcWriteBuffer );

    /* Minus three for the null terminator and half the number of characters in
     * "Task" so the column lines up with the centre of the heading. */
    configASSERT( configMAX_TASK_NAME_LEN > 3 );

    for( xSpacePadding = strlen( "Task" ); xSpacePadding < ( configMAX_TASK_NAME_LEN - 3 ); xSpacePadding++ )
    {
        /* Add a space to align columns after the task's name. */
        *pcWriteBuffer = ' ';
        pcWriteBuffer++;

        /* Ensure always terminated. */
        *pcWriteBuffer = 0x00;
    }

    strcpy( pcWriteBuffer, pcHeader );
    vTaskList( pcWriteBuffer + strlen( pcHeader ) );

    /* There is no more data to return after this single string, so return
     * pdFALSE. */
    return pdFALSE;
}

/* Structure that defines the "task-stats" command line command.  This generates
 * a table that gives information on each task in the system. */
commandREGISTER static const CLI_Command_Definition_t xTaskStats =
{
    "task-stats",        /* The command string to type. */
    "\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
    prvTaskStatsCommand, /* The function to run. */
    0                    /* No parameters are expected. */
};

/*-----------------------------------------------------------*/

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
    int len = snprintf(pcWriteBuffer, xWriteBufferLen, "uid: 0x%llX", board_id);

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
