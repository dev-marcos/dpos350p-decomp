/*
 * channel_traces_process_full -- stock 0x4003C85C (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_process_full for channel ch (0 or 1) when it is on, then for its second trace (ch + 2) when SYS+0x2AA is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void trace_process_full(u32 tr);

void channel_traces_process_full(u32 ch)
{
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        trace_process_full(0);
        if (SYS8(0x2AA))
            trace_process_full(2);
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
        trace_process_full(1);
        if (SYS8(0x2AA))
            trace_process_full(3);
    }
}
