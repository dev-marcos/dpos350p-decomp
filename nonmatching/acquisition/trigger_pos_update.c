/*
 * trigger_pos_update -- stock 0x4003FAB8 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Trigger position = (L + SYS_INFO+0x63C - SYS_INFO+0x6F2) mod L, L = SYS_INFO+0x520, to the FPGA.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"

void trigger_pos_update(void)
{
    u32 L = SYS32(0x520);
    u32 v = L + SYS32(0x63C) - SYS16(0x6F2);
    fpga_trigger_pos_write(v - (v / L) * L);
}
