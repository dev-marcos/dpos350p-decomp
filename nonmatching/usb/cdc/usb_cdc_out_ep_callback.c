/*
 * usb_cdc_out_ep_callback -- stock 0x4006D884 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b usb_cdc_echo.
 */
#include "hw/usb.h"
#include "usb/cdc.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int usb_cdc_out_ep_callback(unsigned char *buf, int len) { return usb_cdc_echo(buf, len); }
