/*
 * usb_mass_out_ep_callback -- stock 0x4006B9F0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b usb_mass_out_handler.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_mass_out_ep_callback(unsigned char *buf, u32 len) { usb_mass_out_handler(buf, len); }
