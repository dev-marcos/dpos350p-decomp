/*
 * usb_device_clear_setup_end -- stock 0x4006AC40 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b USBC_Dev_Ctrl_ClearSetupEnd.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/usbc.h"

void usb_device_clear_setup_end(void) { USBC_Dev_Ctrl_ClearSetupEnd(); }
