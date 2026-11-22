/*
 * usb_mass_disk_write -- stock 0x4006A978 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes through 0x4006A868 when the SD card state is 1; otherwise returns its first argument unchanged. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 usb_disk_sd_write(u32 a, u32 b, u32 c, u32 d);

u32 usb_mass_disk_write(u32 a, u32 b, u32 c)
{
    if (*(volatile u32 *)0x40095490u == 1)
        return usb_disk_sd_write(a, b, c, 0x40095490u);   /* r3 still holds the state address */
    return a;
}
