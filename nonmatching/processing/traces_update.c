/*
 * traces_update -- stock 0x4003C970 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, and unless SYS+0x27A is 1 with SYS+0x3C and SYS+0x38 both 0: channels_process_full, or traces_process_single when SYS+0x652 is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void traces_update(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    if (SI(trigger_mode) == 1 && SI(acq_transition_flag) == 0 && SI(acq_done) == 0)
        return;
    if (SI(f_652) == 0)
        channels_process_full();
    else
        traces_process_single();
}
