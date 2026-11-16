/*
 * traces_process_single -- stock 0x4003C910 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Processing used when SYS+0x652 is set: 0x4005F90C(0/1), 0x4003C0AC(0), 0x4003C158(0/1), 0x4003C314(0/1), 0x4003BB6C(0), 0x4005EDC0(0), 0x4005DDD4(0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_span_full_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void func_4003c158(u32 tr);
void func_4003c314(u32 tr);
void func_4003bb6c(u32 tr);
void func_4005edc0(u32 tr);
void func_4005ddd4(u32 tr);

void traces_process_single(void)
{
    trace_span_full_veneer(0);
    trace_span_full_veneer(1);
    scaled_pair_update(0);
    func_4003c158(0);
    func_4003c158(1);
    func_4003c314(0);
    func_4003c314(1);
    func_4003bb6c(0);
    func_4005edc0(0);
    func_4005ddd4(0);
}
