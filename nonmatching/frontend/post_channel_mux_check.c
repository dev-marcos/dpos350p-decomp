/*
 * post_channel_mux_check -- stock 0x4004AF0C (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * POST channel-multiplex test: both channels on at the given V/div, positions 0xAA then 0x1E; each time the measured mean position (SYS_INFO+0xB34 / +0xB50) must be within +-3 (+-6 at index 12). Returns 1 both pass, -2 CH1 only, -1 CH2 only, 0 neither.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/timer.h"
#include "frontend.h"
#include "system.h"
#include "sys_info.h"

int post_channel_mux_check(u32 vdiv)
{
    u32 tol = vdiv == 12 ? 6 : 3;
    unsigned char ok_hi1 = 0, ok_hi2 = 0, ok_lo1 = 0, ok_lo2 = 0;
    u32 a, m;
    SI(timebase_index) = 0x18; SI(ch1_vdiv_index) = vdiv; SI(ch2_vdiv_index) = vdiv;
    SI(acq_transition_flag) = 0; SI(ch1_enabled) = 1; SI(ch1_probe) = 0; SI(ch1_coupling) = 0; SI(f_046) = 2;
    SI(ch2_enabled) = 1; SI(ch2_probe) = 0; SI(ch2_coupling) = 0; SI(f_10a) = 2; SI(f_275) = 0; SI(hires_option) = 0;
    SI(ch1_offset) = 0xAA; SI(ch2_offset) = 0xAA;
    outputs_apply(); delay_ms(100); acquisition_dispatcher();
    a = SI(ch1_offset); m = SI(ch1_mean_pos);
    if (a - tol < m && m < tol + a) ok_hi1 = 1;
    a = SI(ch2_offset); m = SI(ch2_mean_pos);
    if (a - tol < m && m < tol + a) ok_hi2 = 1;
    SI(ch1_offset) = 0x1E; SI(ch2_offset) = 0x1E;
    outputs_apply(); delay_ms(100); acquisition_dispatcher();
    a = SI(ch1_offset); m = SI(ch1_mean_pos);
    if (a - tol < m && m < tol + a) ok_lo1 = 1;
    a = SI(ch2_offset); m = SI(ch2_mean_pos);
    if (a - tol < m && m < tol + a) ok_lo2 = 1;
    {
        int c1 = (ok_lo1 & ok_hi1) != 0, c2 = (ok_lo2 & ok_hi2) != 0;
        if (c1 & c2) return 1;
        if (c1) return -2;
        if (c2) return -1;
        return 0;
    }
}
