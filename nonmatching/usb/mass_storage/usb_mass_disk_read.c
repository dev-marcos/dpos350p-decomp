/*
 * usb_mass_disk_read -- stock 0x4006A964 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads through 0x4006A738 when the SD card state (0x40095490) is 1; otherwise returns its first argument unchanged. Stock quirks kept: the callee also receives r3 = 0x40095490. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "usb/mass_storage.h"

u32 usb_mass_disk_read(u32 lun, u32 lba, u32 count)
{
    if (*(volatile u32 *)IMG(40095490) == 1)
        return usb_disk_sd_read(lun, lba, count, IMGU(40095490));   /* r3 still holds the state address */
    return lun;
}
