/*
 * applied_state_invalidate -- stock 0x4003EF1C (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the 'last applied' copy at SYS+0xBB0..0xBCA with the complement of the current values (bytes SYS+0x6E4..0x6E7, word +0x6E8, V/div bytes +0x41 / +0x105, byte +0x100) and clears +0xBB8..+0xBC7, so that everything is applied again; then 0x40063BC0 and outputs_apply.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"
#include "system.h"
#include "sys_info.h"

void applied_state_invalidate(void)
{
    SI(f_bb0) = ~SI(f_6e4);
    SI(f_bb1) = ~SI(f_6e5);
    SI(f_bb2) = ~SI(f_6e6);
    SI(f_bb3) = ~SI(f_6e7);
    SI(f_bb4) = ~SI(f_6e8);
    SI(f_bb8) = 0;
    SI(f_bbc) = 0;
    SI(f_bc0) = 0;
    SI(f_bc4) = 0;
    SI(f_bc9) = ~SI(ch2_vdiv_index);
    SI(f_bc8) = ~SI(ch1_vdiv_index);
    SI(f_bca) = ~SI(f_100);
    fpga_counters_init();
    outputs_apply();
}
