/*
 * scsi_check_range -- stock 0x4006DCE4 (156 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * lba + count must fit the LUN (else LBA OUT OF RANGE, ASC 0x21) and the CBW length must equal count * block size (else ASC 0x24). Returns 1 when valid.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

int scsi_check_range(u32 lun, u32 op, u32 lba, u32 count)
{
    (void)op;
    if (lun_blocks[lun] < lba + count) {
        usb_mass_set_sense(lun, 5, 0x21);
        usb_mass_send_csw(1, 0);
        return 0;
    }
    if (CBW_DLEN != lun_blksize[lun] * count) {
        usb_mass_set_sense(CBW_LUN, 5, 0x24);
        usb_mass_send_csw(1, 0);
        return 0;
    }
    return 1;
}
