/*
 * usb_device_read_data_status_ep0 -- stock 0x4006AB24 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0 read status.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usb_device_read_data_status_ep0(u32 is_complete) { USBC_Dev_ReadDataStatus(USBC_EP_TYPE_EP0, is_complete); }
