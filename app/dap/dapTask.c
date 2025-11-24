#include "dapTask.h"


/*-----------------------------------------------------------*/

/// @brief dap link thread
/// @param pv : dap task interface
void vdapTask(void * pv)
{
    DAP_Setup();
    
    dap_t * px = (dap_t *)pv;

    uint8_t rx[DAP_BUFFER_SIZE] = { 0 };
    uint8_t tx[DAP_BUFFER_SIZE] = { 0 };

    do
    {
        uint32_t response_size = px->r(rx, (int)DAP_BUFFER_SIZE);
        DAP_ExecuteCommand(rx, tx);
        px->w(tx, response_size);
    } while (true);
    
}

/*-----------------------------------------------------------*/