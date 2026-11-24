/*
 * scsi_lun_invalid -- stock 0x4006D888 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * LUN > 1.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int scsi_lun_invalid(u32 lun) { return lun > 1; }
