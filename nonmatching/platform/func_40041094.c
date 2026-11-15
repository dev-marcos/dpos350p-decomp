/*
 * channel_to_display_copy -- stock 0x40041094 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies SYS+0x454[k] samples of the channel buffer into the display buffer 0x42AC7CF4 + ch x 0x62250 at index SYS+0x2C4[k].
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void channel_to_display_copy(u32 ch, u32 k)
{
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
    }

    copy_u16((unsigned short *)(0x42AC7CF4u + ch * 0x62250u + SYS32(0x2C4 + 4 * k) * 2),
             (const unsigned short *)SYS32(0x7B4 + 4 * ch), SYS32(0x454 + 4 * k));
}
