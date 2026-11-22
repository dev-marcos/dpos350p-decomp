/*
 * trace_c5cc_window -- stock 0x4003C7A0 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x6FC >= SYS+0x708: 0x4003C5CC(tr, (u16)SYS+0x700, SYS16+0x6F8).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void trace_c5cc_window(u32 tr)
{
    if (SYS32(0x6FC) < SYS32(0x708))
        return;
    trace_segments_gather(tr, SYS32(0x700) & 0xFFFF, SYS16(0x6F8));
}
