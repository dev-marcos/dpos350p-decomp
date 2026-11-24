/*
 * usb_mass_max_lun_ptr -- stock 0x4006B978 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Address of the max-LUN byte (0x42E68D23).
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

unsigned char *usb_mass_max_lun_ptr(void) { return (unsigned char *)0x42E68D23u; }
