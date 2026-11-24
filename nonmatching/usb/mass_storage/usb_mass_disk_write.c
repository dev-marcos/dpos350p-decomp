/*
 * usb_mass_disk_write -- stock 0x4006A978 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes through 0x4006A868 when the SD card state is 1; otherwise returns its first argument unchanged. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "usb/mass_storage.h"

u32 usb_mass_disk_write(u32 lun, u32 lba, u32 count)
{
    if (*(volatile u32 *)0x40095490u == 1)
        return usb_disk_sd_write(lun, lba, count, 0x40095490u);   /* r3 still holds the state address */
    return lun;
}
