/*
 * trace_process_full -- stock 0x4003C7E4 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Full processing of trace tr, run only when SYS+0x101 is set and, for a 0x4003BF78(tr) result of 1, SYS+0x2AA is set too: 0x4005F90C, 0x4003C0AC, 0x4003C158, 0x4003C314, 0x4003BB6C, trace_c5cc_full, 0x4005E12C, 0x4005DDD4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 func_4003bf78(u32 tr);
void func_4005f90c(u32 tr);
void func_4003c0ac(u32 tr);
void func_4003c158(u32 tr);
void func_4003c314(u32 tr);
void func_4003bb6c(u32 tr);
void trace_c5cc_full(u32 tr);
void func_4005e12c(u32 tr);
void func_4005ddd4(u32 tr);

void trace_process_full(u32 tr)
{
    u32 k = func_4003bf78(tr);
    if (SYS8(0x101) == 0)
        return;
    if (k == 1 && SYS8(0x2AA) == 0)
        return;
    func_4005f90c(tr);
    func_4003c0ac(tr);
    func_4003c158(tr);
    func_4003c314(tr);
    func_4003bb6c(tr);
    trace_c5cc_full(tr);
    func_4005e12c(tr);
    func_4005ddd4(tr);
}
