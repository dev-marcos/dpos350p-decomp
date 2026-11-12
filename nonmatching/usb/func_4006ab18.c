/*
 * usb_device_send_nullpack_ep0 -- stock 0x4006AB18 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * EP0 write status, complete.
 */
#include "hw/usb.h"

void usb_device_send_nullpack_ep0(void) { USBC_Dev_WriteDataStatus(USBC_EP_TYPE_EP0, 1); }
