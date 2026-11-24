/*
 * usb_mass_send_data -- stock 0x4006C04C (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Data-in phase: send on EP1, stage 3 (CSW next), status 0, residue -= len.
 */
#include "hw/usb.h"
#include "usb/core.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_mass_send_data(unsigned char *buf, int len)
{
    usb_device_write_data(1, buf, len);
    mass_stage = 3;
    csw[12] = 0;
    CSW_RESIDUE -= len;
}
