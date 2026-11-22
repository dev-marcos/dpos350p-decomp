/*
 * channels_process_full -- stock 0x4003C8D0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * channels_process_full_ordered(SYS_INFO). The stock entry is one instruction that falls through into 0x4003C8D4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void channels_process_full(void)
{
    channels_process_full_ordered((volatile unsigned char *)0x40544E68u);
}
