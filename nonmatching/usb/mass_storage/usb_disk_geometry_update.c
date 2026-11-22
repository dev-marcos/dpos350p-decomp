/*
 * usb_disk_geometry_update -- stock 0x4006A98C (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * USB disk geometry: 0x42E68D0C = sector size (0x4009548C); when the SD state is 1 the byte capacity 0x42E68C60 = sectors (0x42E68A18+4) x size; 0x42E68D10 = capacity / size (64-bit).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d);

void usb_disk_geometry_update(void)
{
    u32 ss = *(volatile u32 *)0x4009548Cu;
    volatile unsigned long long *cap = (volatile unsigned long long *)0x42E68C60u;
    *(volatile u32 *)0x42E68D0Cu = ss;
    if (*(volatile u32 *)0x40095490u == 1)
        *cap = (unsigned long long)*(volatile u32 *)(0x42E68A18u + 4) * ss;
    *(volatile u32 *)0x42E68D10u = (u32)__aeabi_uldivmod(*cap, ss);
}
