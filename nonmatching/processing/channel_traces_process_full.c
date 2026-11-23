/*
 * channel_traces_process_full -- stock 0x4003C85C (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_process_full for channel ch (0 or 1) when it is on, then for its second trace (ch + 2) when SYS+0x2AA is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void channel_traces_process_full(u32 ch)
{
    if (ch == 0) {
        if (SI(ch1_enabled) == 0)
            return;
        trace_process_full(0);
        if (SI(zoom_on))
            trace_process_full(2);
    } else if (ch == 1) {
        if (SI(ch2_enabled) == 0)
            return;
        trace_process_full(1);
        if (SI(zoom_on))
            trace_process_full(3);
    }
}
