/*
 * per_channel_processing -- stock 0x4003C9F4 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_process_window for CH1 and CH2 when they are on. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void per_channel_processing(void)
{
    if (SI(ch1_enabled))
        trace_process_window(0);
    if (SI(ch2_enabled))
        trace_process_window(1);
}
