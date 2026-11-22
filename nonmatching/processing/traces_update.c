/*
 * traces_update -- stock 0x4003C970 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, and unless SYS+0x27A is 1 with SYS+0x3C and SYS+0x38 both 0: channels_process_full, or traces_process_single when SYS+0x652 is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void channels_process_full(void);
void traces_process_single(void);

void traces_update(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    if (SYS8(0x27A) == 1 && SYS8(0x3C) == 0 && SYS8(0x38) == 0)
        return;
    if (SYS8(0x652) == 0)
        channels_process_full();
    else
        traces_process_single();
}
