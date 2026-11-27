/*
 * scsi_read10 -- stock 0x4006DA48 (196 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * READ(10): stage 0 validates (range, direction) and starts; stage 2 continues. Data comes from the disk layer (0x4006A964).
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_read10(u32 lun, u32 lba, u32 count)
{
    if (mass_stage == 0) {
        if (!scsi_check_range(CBW_LUN, 0x28, lba, count)) {
            debug_printf_stub(STR(4008d9cc));
            return;
        }
        if (!(CBW_FLAGS & 0x80)) {
            debug_printf_stub(STR(4008e8bc));
            usb_mass_set_sense(CBW_LUN, 5, 0x24);
            usb_mass_send_csw(1, 1);
            return;
        }
        debug_printf_stub(STR(4008ffa8));
        mass_stage = 2;
    } else if (mass_stage == 2) {
        debug_printf_stub(STR(4008ffa8));
    } else {
        return;
    }
    usb_mass_disk_read(lun, lba, count);
}
