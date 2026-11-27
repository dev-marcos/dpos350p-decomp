/*
 * ff_uni2oem -- stock 0x40069D90 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unicode to OEM code (FatFs ffunicode.c, CP437 only): ASCII passes, 0x80..0xFF is looked up, anything else or another code page gives 0. An unmatched character also gives 0 ((0x80 + 0x80) & 0xFF). (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define CP437_HI   ((const volatile unsigned short *)IMG(4008c50c))   /* Unicode of OEM 0x80..0xFF */

u32 ff_uni2oem(u32 uni, u32 cp)
{
    u32 c;
    if (uni < 0x80)
        return uni & 0xFFFF;
    if (uni >= 0x100 || cp != 437)
        return 0;
    for (c = 0; c < 0x80; c++)
        if (CP437_HI[c] == uni)
            break;
    return (c + 0x80) & 0xFF;
}
