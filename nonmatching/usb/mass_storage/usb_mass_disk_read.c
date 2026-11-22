/*
 * usb_mass_disk_read -- stock 0x4006A964 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads through 0x4006A738 when the SD card state (0x40095490) is 1; otherwise returns its first argument unchanged. Stock quirks kept: the callee also receives r3 = 0x40095490. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "usb/mass_storage.h"

u32 usb_mass_disk_read(u32 a, u32 b, u32 c)
{
    if (*(volatile u32 *)0x40095490u == 1)
        return usb_disk_sd_read(a, b, c, 0x40095490u);   /* r3 still holds the state address */
    return a;
}
