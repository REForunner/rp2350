#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
// Include sys/types.h before inttypes.h to work around issue with
// certain versions of GCC and newlib which causes omission of PRIx64
#include <sys/types.h>
#include <inttypes.h>   // for PRIx macros if needed
#include "pico/rand.h"

// Idle task
static StaticTask_t xIdleTCB;
static StackType_t  xIdleStack[configMINIMAL_STACK_SIZE];
// CPU running time statistics
volatile uint32_t ulHighFrequencyTimerTicks;

void vApplicationGetIdleTaskMemory( StaticTask_t **ppTCB,
                                    StackType_t **ppStack,
                                    uint32_t     *pSize )
{
    *ppTCB   = &xIdleTCB;
    *ppStack = xIdleStack;
    *pSize   = configMINIMAL_STACK_SIZE;
}

// Passive idle task (SMP)
#if ( configNUMBER_OF_CORES > 1 )
static StaticTask_t xPassiveTCB;
static StackType_t  xPassiveStack[configMINIMAL_STACK_SIZE];

void vApplicationGetPassiveIdleTaskMemory( StaticTask_t **ppTCB,
                                           StackType_t **ppStack,
                                           uint32_t     *pSize,
                                           BaseType_t    xCoreID )
{
    (void)xCoreID;
    *ppTCB   = &xPassiveTCB;
    *ppStack = xPassiveStack;
    *pSize   = configMINIMAL_STACK_SIZE;
}
#endif

// Timer task
static StaticTask_t xTimerTCB;
static StackType_t  xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppTCB,
                                     StackType_t **ppStack,
                                     uint32_t     *pSize )
{
    *ppTCB   = &xTimerTCB;
    *ppStack = xTimerStack;
    *pSize   = configTIMER_TASK_STACK_DEPTH;
}

// Error hooks
void vApplicationStackOverflowHook( TaskHandle_t hTask,
                                    char        *pcTaskName )
{
    printf("\n*** STACK OVERFLOW [%s] ***\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for(;;);
}

void vApplicationMallocFailedHook( void )
{
    printf("\n*** MALLOC FAILED ***\n");
    taskDISABLE_INTERRUPTS();
    for(;;);
}

void vAssertCalled( const char *file, uint32_t line )
{
    printf("\n*** ASSERT FAILED %s:%" PRIu32 " ***\n", file, line);
    taskDISABLE_INTERRUPTS();
    for(;;);
}

#if ( configENABLE_HEAP_PROTECTOR == 1 )

/**
 * @brief Application provided function to get a random value to be used as canary.
 *
 * @param pxHeapCanary [out] Output parameter to return the canary value.
 */
void vApplicationGetRandomHeapCanary( portPOINTER_SIZE_TYPE * pxHeapCanary ) { * pxHeapCanary = (portPOINTER_SIZE_TYPE)get_rand_32(); }

#endif /* configENABLE_HEAP_PROTECTOR */