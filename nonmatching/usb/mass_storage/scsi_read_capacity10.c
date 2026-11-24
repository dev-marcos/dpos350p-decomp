/*
 * scsi_read_capacity10 -- stock 0x4006D95C (160 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * READ CAPACITY(10): last LBA and block size, big endian, at 0x42E68C48.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "usb/phy.h"
#include "shared/usb.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void usb_phy_nop_raw(u32 x) __asm__("usb_phy_nop");

void scsi_read_capacity10(u32 lun)
{
    unsigned char *b = (unsigned char *)0x42E68C48u;
    u32 n, s;
    if (scsi_lun_invalid(lun)) {
        usb_mass_set_sense(CBW_LUN, 2, 0x3A);   /* NOT READY, MEDIUM NOT PRESENT */
        usb_mass_send_csw(1, 1);
        usb_phy_nop_raw(0);
        return;
    }
    n = lun_blocks[lun] - 1;
    b[0] = (unsigned char)(n >> 24); b[1] = (unsigned char)(n >> 16); b[2] = (unsigned char)(n >> 8); b[3] = (unsigned char)n;
    s = lun_blksize[lun];
    b[4] = (unsigned char)(s >> 24); b[5] = (unsigned char)(s >> 16); b[6] = (unsigned char)(s >> 8); b[7] = (unsigned char)s;
    usb_mass_send_data(b, 8);
}
