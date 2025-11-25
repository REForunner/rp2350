#include "DAP.h"
#include "rp2350.h"

#ifndef DAP_TASK_H_
#define DAP_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#if CFG_TUD_HID

/*-----------------------------------------------------------*/

/* dap task buffer size */
#define DAP_BUFFER_SIZE     (CFG_TUD_HID_EP_BUFSIZE + 0x10)

/*-----------------------------------------------------------*/

/* Prototype of data interaction function */
typedef int (* DAP_DataInteraction_t)(uint8_t *, int);
/* swj sequence */
typedef void (* SWJ_Sequence_t)(unsigned int, const uint8_t *);
/* swd sequenct */
typedef void (* SWD_Sequence_t)(unsigned int, const uint8_t *, uint8_t *);
/* swd transmit */
typedef uint8_t (* SWD_Transfer_t)(unsigned int, unsigned int *);

/* dap task interface */
typedef struct dap_t
{
    DAP_DataInteraction_t r;    // read, if return -1, it represents overflow.
    DAP_DataInteraction_t w;    // write
    SWJ_Sequence_t swj;
    SWD_Sequence_t swd;
    SWD_Transfer_t swd_transfer;
} dap_t;

/*-----------------------------------------------------------*/

/// @brief dap link thread
/// @param pv : dap task interface
void vdapTask(void * pv);

/*-----------------------------------------------------------*/

#endif

#endif  /* DAP_TASK_H_ */

#ifdef __cplusplus
}

#endif