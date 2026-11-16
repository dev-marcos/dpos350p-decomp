/*
 * trace_span_window -- stock 0x4005F8E8 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4005F778(tr, SYS+0x6FC, SYS+0x6F6 (16 bit)).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4005f778(u32 tr, u32 start, u32 len);

void trace_span_window(u32 tr)
{
    func_4005f778(tr, SYS32(0x6FC), SYS16(0x6F6));
}
