/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tusb_edpt_handler.h"
#include "dap/DAP_config.h"
#include "dap/DAP.h"
#include "rp2350.h"
#include "FreeRTOS.h"
#include "task.h"


static uint8_t itf_num;
static uint8_t _rhport;

static uint8_t _out_ep_addr;
static uint8_t _in_ep_addr;

static uint8_t requestBuffer[DAP_PACKET_SIZE];
static uint8_t responseBuffer[DAP_PACKET_SIZE];

bool is_in_isr(void) {
    // xPortIsInsideInterrupt(): return true is in isr
    return xPortIsInsideInterrupt() != pdFALSE;
}

void dap_edpt_init(void) {

}

bool dap_edpt_deinit(void)
{
	return true;
}

void dap_edpt_reset(uint8_t __unused rhport)
{
	itf_num = 0;
}

char * dap_cmd_string[] = {
	[ID_DAP_Info               ] = "DAP_Info",
	[ID_DAP_HostStatus         ] = "DAP_HostStatus",
	[ID_DAP_Connect            ] = "DAP_Connect",
	[ID_DAP_Disconnect         ] = "DAP_Disconnect",
	[ID_DAP_TransferConfigure  ] = "DAP_TransferConfigure",
	[ID_DAP_Transfer           ] = "DAP_Transfer",
	[ID_DAP_TransferBlock      ] = "DAP_TransferBlock",
	[ID_DAP_TransferAbort      ] = "DAP_TransferAbort",
	[ID_DAP_WriteABORT         ] = "DAP_WriteABORT",
	[ID_DAP_Delay              ] = "DAP_Delay",
	[ID_DAP_ResetTarget        ] = "DAP_ResetTarget",
	[ID_DAP_SWJ_Pins           ] = "DAP_SWJ_Pins",
	[ID_DAP_SWJ_Clock          ] = "DAP_SWJ_Clock",
	[ID_DAP_SWJ_Sequence       ] = "DAP_SWJ_Sequence",
	[ID_DAP_SWD_Configure      ] = "DAP_SWD_Configure",
	[ID_DAP_SWD_Sequence       ] = "DAP_SWD_Sequence",
	[ID_DAP_JTAG_Sequence      ] = "DAP_JTAG_Sequence",
	[ID_DAP_JTAG_Configure     ] = "DAP_JTAG_Configure",
	[ID_DAP_JTAG_IDCODE        ] = "DAP_JTAG_IDCODE",
	[ID_DAP_SWO_Transport      ] = "DAP_SWO_Transport",
	[ID_DAP_SWO_Mode           ] = "DAP_SWO_Mode",
	[ID_DAP_SWO_Baudrate       ] = "DAP_SWO_Baudrate",
	[ID_DAP_SWO_Control        ] = "DAP_SWO_Control",
	[ID_DAP_SWO_Status         ] = "DAP_SWO_Status",
	[ID_DAP_SWO_ExtendedStatus ] = "DAP_SWO_ExtendedStatus",
	[ID_DAP_SWO_Data           ] = "DAP_SWO_Data",
	[ID_DAP_QueueCommands      ] = "DAP_QueueCommands",
	[ID_DAP_ExecuteCommands    ] = "DAP_ExecuteCommands",
};


uint16_t dap_edpt_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{

	TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
			DAP_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
			DAP_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol, 0);

	uint16_t const drv_len = sizeof(tusb_desc_interface_t) + (itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
	TU_VERIFY(max_len >= drv_len, 0);
	itf_num = itf_desc->bInterfaceNumber;
	_rhport = rhport;

	// Iterate endpoints and open them based on direction (some hosts may list IN/OUT in any order)
	tusb_desc_endpoint_t *edpt_desc = (tusb_desc_endpoint_t *) (itf_desc + 1);
	_out_ep_addr = 0;
	_in_ep_addr = 0;

	for (uint8_t i = 0; i < itf_desc->bNumEndpoints; i++, edpt_desc++)
	{
		uint8_t ep_addr = edpt_desc->bEndpointAddress;
		// open endpoint in tinyusb
		usbd_edpt_open(rhport, edpt_desc);

		if (tu_edpt_dir(ep_addr) == TUSB_DIR_OUT)
		{
			_out_ep_addr = ep_addr;
			// start OUT transfer so stack can receive data into our buffer
			usbd_edpt_xfer(rhport, ep_addr, requestBuffer, DAP_PACKET_SIZE);
		}
		else if (tu_edpt_dir(ep_addr) == TUSB_DIR_IN)
		{
			_in_ep_addr = ep_addr;
			// IN endpoint: do not queue transfer here, main dap thread will send when data ready
		}
	}

	// Ensure both endpoints found
	TU_VERIFY(_out_ep_addr != 0 && _in_ep_addr != 0, 0);

	return drv_len;

}

bool dap_edpt_control_xfer_cb(uint8_t __unused rhport, uint8_t stage, tusb_control_request_t const *request)
{
	return false;
}

// Manage responseBuffer (request) write and requestBuffer (response) read indices
bool dap_edpt_xfer_cb(uint8_t __unused rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
	const uint8_t ep_dir = tu_edpt_dir(ep_addr);

	/* to pc(send) */
	if(ep_dir == TUSB_DIR_IN)
	{
		// IN transfer completed. Only act on successful transfers.
		if (result == XFER_RESULT_SUCCESS && xferred_bytes > 0u && xferred_bytes <= DAP_PACKET_SIZE)
		{
			// nothing to do here: dap_thread triggers IN transfers when it has data to send
			return true;
		}
		return (result == XFER_RESULT_SUCCESS);
	}
	/* to self(receive) */
	else if(ep_dir == TUSB_DIR_OUT)
	{

		// Only process successful OUT transfers with a valid byte count
		if (result == XFER_RESULT_SUCCESS && xferred_bytes > 0u && xferred_bytes <= DAP_PACKET_SIZE)
		{
			// copy received data into the stream buffer for dap_thread
			xStreamBufferSend(dapStreambuf, requestBuffer, xferred_bytes, 0);
			// re-arm OUT endpoint to receive next packet
			usbd_edpt_xfer(rhport, ep_addr, requestBuffer, DAP_PACKET_SIZE);
			return true;
		}
		else if (result == XFER_RESULT_SUCCESS && xferred_bytes == 0u)
		{
			// zero-length packet received (possible), re-arm to continue receiving
			usbd_edpt_xfer(rhport, ep_addr, requestBuffer, DAP_PACKET_SIZE);
			return true;
		}
		return false;
	}
	else 
	{
		return false;
	}
}

void dap_thread(void *ptr)
{
	uint32_t n;
	//  Initialise buffer indices
	uint8_t DAPRequestBuffer[DAP_PACKET_SIZE];
	uint8_t DAPResponseBuffer[DAP_PACKET_SIZE];
	
	do
	{
		uint32_t _resp_len;
		_resp_len = (uint32_t)xStreamBufferReceive(dapStreambuf, DAPRequestBuffer, DAP_PACKET_SIZE, portMAX_DELAY);
		_resp_len = DAP_ExecuteCommand(DAPRequestBuffer, DAPResponseBuffer);
		memcpy(responseBuffer, DAPResponseBuffer, (uint16_t) _resp_len);
		usbd_edpt_xfer(_rhport, _in_ep_addr, responseBuffer, (uint16_t) _resp_len);
	} while (true);

}

usbd_class_driver_t const _dap_edpt_driver =
{
		.init = dap_edpt_init,
		.deinit = dap_edpt_deinit,
		.reset = dap_edpt_reset,
		.open = dap_edpt_open,
		.control_xfer_cb = dap_edpt_control_xfer_cb,
		.xfer_cb = dap_edpt_xfer_cb,
		.sof = NULL,
#if CFG_TUSB_DEBUG >= 2
		.name = "DAP ENDPOINT"
#endif
};

// Add the custom driver to the tinyUSB stack
usbd_class_driver_t const * usbd_app_driver_get_cb(uint8_t *driver_count)
{
	* driver_count = 1;
	return &_dap_edpt_driver;
}

