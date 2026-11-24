/*
 * usb_mass_in_handler -- stock 0x4006B9F4 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * IN state machine: stage 2 continues a READ(10), stage 3 sends the CSW, stages 4/5 return to CBW.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_mass_in_handler(void)
{
    u32 fl = mass_flags | 0x10;
    mass_flags = (unsigned char)fl;
    switch (mass_stage) {
    case 2:
        if (CBW_OP != 0x28) return;
        mass_flags = (unsigned char)(fl | 2);
        scsi_read10(CBW_LUN, mass_lba, mass_count);
        return;
    case 3:
        usb_mass_send_csw(0, 1);
        return;
    case 4:
    case 5:
        mass_stage = 0;
        return;
    }
}
