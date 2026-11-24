/*
 * acquisition_geometry_setup -- stock 0x4003C008 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sets the capture geometry: SYS+0x38 = 1; step SYS16+0x6F8 = 2 at time-base index 0x21, else 1; SYS16+0x6F6 = SYS+0x558 x step; per-bank count SYS16+0x6F4 = SYS16+0x6F6 / SYS+0x1B1 + 2 (copied to +0x640 and +0x6F0); SYS+0x708 = SYS16+0x6F6 + 1; SYS+0x454 = SYS+0x2B0, SYS+0x38C = SYS+0x2B0 / 2; SYS+0x568 / +0x56C = -SYS16+0xC; clears SYS+0x6FC, +0x700, +0x2C4, +0x63C. The stock division by zero gives 0; the C does the same explicitly.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void acquisition_geometry_setup(void)
{
    u32 d, q, prod, L;
    int b;
    SI(acq_done) = 1;
    d = SI(repeat_factor);
    SI(runtime_coarse_step) = SI(timebase_index) == 0x21 ? 2 : 1;
    prod = (SI(bank_count) * SI(runtime_coarse_step)) & 0xFFFF;
    SI(runtime_step) = prod;
    b = (int)d;
    q = b ? (u32)((int)prod / b) : 0;
    SI(runtime_field_6f4) = q + 2;
    SI(runtime_fine_progress) = 0;
    SI(runtime_coarse_progress) = 0;
    SI(f_640) = SI(runtime_field_6f4);
    SI(runtime_fpga_position) = SI(runtime_field_6f4);
    L = SI(block_points);
    SI(block_start) = 0;
    SI(runtime_quotient) = 0;
    SI(runtime_progress_threshold) = SI(runtime_step) + 1;
    SI(block_len) = L;
    SI(block_base) = L >> 1;
    SI(f_568) = -(u32)SI(plot_x_mid);
    SI(f_56c) = -(u32)SI(plot_x_mid);
}
