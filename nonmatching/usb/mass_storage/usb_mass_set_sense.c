/*
 * usb_mass_set_sense -- stock 0x4006DA2C (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * sense_data[2] = key, [12] = ASC (the LUN is ignored).
 */
#include "hw/usb.h"
#include "usb/mass_storage.h"
#include "shared/usb.h"

void usb_mass_set_sense(u32 lun, u32 key, u32 asc)
{
    (void)lun;
    sense_data[2] = (unsigned char)key;
    sense_data[12] = (unsigned char)asc;
}
