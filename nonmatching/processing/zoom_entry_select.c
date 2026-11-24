/*
 * zoom_entry_select -- stock 0x40041FF8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the 64-bit entry SYS+0xBD0 + 8 x SYS+0x598 to SYS+0x5A0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void zoom_entry_select(void)
{
    volatile u32 *s = &SYS32(0xBD0 + 8 * SI(zoom_index));
    u32 lo = s[0], hi = s[1];
    SYS32(0x5A0) = lo;
    SYS32(0x5A4) = hi;
}
