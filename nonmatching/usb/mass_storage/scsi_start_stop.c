/*
 * scsi_start_stop -- stock 0x4006DA3C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * START STOP UNIT / PREVENT ALLOW MEDIUM REMOVAL: good status.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_start_stop(u32 lun) { (void)lun; usb_mass_send_csw(0, 1); }
