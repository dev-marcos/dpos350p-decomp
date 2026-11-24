/*
 * scsi_format_unit -- stock 0x4006DC44 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FORMAT UNIT: nothing for a valid LUN (no CSW), NOT READY otherwise.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "usb/phy.h"
#include "shared/usb.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void usb_phy_nop_raw(u32 x) __asm__("usb_phy_nop");

void scsi_format_unit(u32 lun)
{
    if (scsi_lun_invalid(lun)) {
        usb_mass_set_sense(CBW_LUN, 2, 0x3A);   /* NOT READY, MEDIUM NOT PRESENT */
        usb_mass_send_csw(1, 1);
        usb_phy_nop_raw(0);
        return;
    }
}
