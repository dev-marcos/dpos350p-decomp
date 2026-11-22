/*
 * channels_process_full_ordered -- stock 0x4003C8D4 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Both channels through channel_traces_process_full; CH2 first when the byte p+0x664 is 0, else CH1 first.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void channels_process_full_ordered(volatile unsigned char *p)
{
    if (p[0x664] == 0) {
        channel_traces_process_full(1);
        channel_traces_process_full(0);
    } else {
        channel_traces_process_full(0);
        channel_traces_process_full(1);
    }
}
