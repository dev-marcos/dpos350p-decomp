/*
 * scsi_write10 -- stock 0x4006DB0C (152 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * WRITE(10): stage 0 validates and waits for data (stage 1); stage 1 passes data to the disk layer (0x4006A978).
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_write10(u32 lun, u32 lba, u32 count)
{
    if (mass_stage == 0) {
        if (!scsi_check_range(CBW_LUN, 0x2A, lba, count))
            return;
        if (!(CBW_FLAGS & 0x80)) {
            mass_stage = 1;
            return;
        }
        usb_mass_set_sense(CBW_LUN, 5, 0x24);
        usb_mass_send_csw(1, 0);
        return;
    }
    if (mass_stage == 1)
        usb_mass_disk_write(lun, lba, count);
}
