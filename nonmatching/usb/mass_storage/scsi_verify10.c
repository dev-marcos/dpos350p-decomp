/*
 * scsi_verify10 -- stock 0x4006DBA4 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * VERIFY(10): good status unless data is expected or BYTCHK is set.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_verify10(u32 lun)
{
    (void)lun;
    if (CBW_DLEN == 0 && !(cbw[0x10] & 4)) {
        usb_mass_send_csw(0, 1);
        return;
    }
    usb_mass_set_sense(CBW_LUN, 5, 0x24);
    usb_mass_send_csw(1, 0);
}
