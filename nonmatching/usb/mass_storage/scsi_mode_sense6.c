/*
 * scsi_mode_sense6 -- stock 0x4006D9FC (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MODE SENSE(6): 4 bytes at 0x40095494.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_mode_sense6(u32 lun) { (void)lun; usb_mass_send_data((unsigned char *)0x40095494u, 4); }
