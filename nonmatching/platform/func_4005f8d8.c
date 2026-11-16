/*
 * trace_span_full -- stock 0x4005F8D8 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4005F778(tr, 0, SYS+0x2C0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4005f778(u32 tr, u32 start, u32 len);

void trace_span_full(u32 tr)
{
    func_4005f778(tr, 0, SYS32(0x2C0));
}
