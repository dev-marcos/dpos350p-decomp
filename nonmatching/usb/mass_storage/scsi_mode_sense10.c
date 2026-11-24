/*
 * scsi_mode_sense10 -- stock 0x4006DA08 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MODE SENSE(10): 8 bytes at 0x4009547C.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_mode_sense10(u32 lun) { (void)lun; usb_mass_send_data((unsigned char *)0x4009547Cu, 8); }
