/*
 * usb_mass_send_csw -- stock 0x4006C08C (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Send the 13-byte CSW with this status; next stage 4 (next != 0) or 5.
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_mass_send_csw(u32 status, u32 next)
{
    csw[12] = (unsigned char)status;
    CSW_SIG = 0x53425355u;
    usb_mass_write(1, csw, 13);
    mass_stage = next ? 4 : 5;
}
