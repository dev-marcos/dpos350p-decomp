/*
 * usb_cdc_out_ep_callback -- stock 0x4006D884 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b usb_cdc_echo.
 */
#include "hw/usb.h"
#include "usb/cdc.h"
#include "usb/mass_storage.h"

#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)

int usb_cdc_out_ep_callback(unsigned char *buf, int len) { return usb_cdc_echo(buf, len); }
