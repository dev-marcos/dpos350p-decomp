/*
 * scsi_inquiry -- stock 0x4006D898 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * INQUIRY: EVPD -> 5 bytes at 0x42E68C40; else the standard data of LUN 0 or the other LUN, at most 36 bytes.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void scsi_inquiry(u32 lun)
{
    u32 n;
    if (cbw[0x10] & 1) {
        usb_mass_send_data((unsigned char *)0x42E68C40u, 5);
        return;
    }
    n = cbw[0x13];
    if ((int)n >= 0x25) n = 0x24;
    usb_mass_send_data(lun == 0 ? (unsigned char *)0x400952F8u : (unsigned char *)0x4009531Cu, n);
}
