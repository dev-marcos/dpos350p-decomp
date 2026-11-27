/*
 * scsi_read_format_capacities -- stock 0x4006D8CC (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * READ FORMAT CAPACITIES: 12 bytes at 0x40095464 (block count, block size, big endian).
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "usb/phy.h"
#include "shared/usb.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void usb_phy_nop_raw(u32 x) __asm__("usb_phy_nop");

void scsi_read_format_capacities(u32 lun)
{
    unsigned char *b = (unsigned char *)IMG(40095464);
    u32 n, s;
    if (scsi_lun_invalid(lun)) {
        usb_mass_set_sense(CBW_LUN, 2, 0x3A);   /* NOT READY, MEDIUM NOT PRESENT */
        usb_mass_send_csw(1, 1);
        usb_phy_nop_raw(0);
        return;
    }
    n = lun_blocks[lun];
    b[7] = (unsigned char)n; b[4] = (unsigned char)(n >> 24); b[5] = (unsigned char)(n >> 16); b[6] = (unsigned char)(n >> 8);
    s = lun_blksize[lun];
    b[9] = (unsigned char)(s >> 16); b[11] = (unsigned char)s; b[10] = (unsigned char)(s >> 8);
    usb_mass_send_data(b, 12);
}
