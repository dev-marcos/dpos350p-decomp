/*
 * trace_process_window -- stock 0x4003C9BC (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Processing of trace tr over the capture window: 0x4005F910, 0x4003C0AC, 0x4003BB6C, trace_c5cc_window, 0x4005E838, 0x4005DF5C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_span_window_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void func_4003bb6c(u32 tr);
void trace_c5cc_window(u32 tr);
void func_4005e838(u32 tr);
void func_4005df5c(u32 tr);

void trace_process_window(u32 tr)
{
    trace_span_window_veneer(tr);
    scaled_pair_update(tr);
    func_4003bb6c(tr);
    trace_c5cc_window(tr);
    func_4005e838(tr);
    func_4005df5c(tr);
}
