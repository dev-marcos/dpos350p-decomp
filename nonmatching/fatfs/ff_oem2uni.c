/*
 * ff_oem2uni -- stock 0x40069DF4 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * OEM code to Unicode (CP437 only). Codes below 0x80, compared signed, are returned unchanged and without truncation. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define CP437_HI   ((const volatile unsigned short *)IMG(4008c50c))   /* Unicode of OEM 0x80..0xFF */

u32 ff_oem2uni(int oem, u32 cp)
{
    if (oem < 0x80)
        return oem;
    if (cp == 437 && oem < 0x100)
        return CP437_HI[oem - 0x80];
    return 0;
}
