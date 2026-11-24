/*
 * acq_defaults_reset -- stock 0x4003EB40 (488 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Resets the acquisition state in SYS_INFO to its defaults and ends with runtime_state_commit_force (tail call). Notable values: time-base index SYS+0x588 = 0x17 and +0x598 = 0x16, the 64-bit words +0x570 / +0x578 = 0, range bytes +0x41 / +0x105 = 6, coupling bytes +0x46 / +0x10A = 0, the trigger level copies +0x27E, +0x60 and +0x120 = u16 +0xE, the cursor words +0x808 = u16 +8 / 5, +0x80C = 4 x (u16 +8) / 5, +0x810 = u16 +0xA / 4, +0x814 = 3 x (u16 +0xA) / 4, the saved-trace state SYS+0x3DA52C (byte 0, u16 +4 = 0, +8 = u16 +8 / 3, +0xC = 2 x (u16 +8) / 3), key repeat counters +0x86A..+0x870 = 0, 0, 1, 1; touch byte +0x16 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define D64(a) (*(volatile unsigned long long *)(a))

void acq_defaults_reset(void)
{
    u32 lvl = SI(level_default), w8, wa, t;
    SI(sample_clock_hz) = 0;
    SI(timebase_index) = 0x17;
    SI(zoom_index) = 0x16;
    W32(SYS + 0x570) = 0;
    W32(SYS + 0x574) = 0;
    D64(SYS + 0x578) = 0;
    SI(acq_transition_flag) = 0;
    SI(zoom_on) = 0;
    SI(ch1_enabled) = 1;
    SI(trigger_level) = (unsigned short)lvl;
    SI(ch1_fft_enabled) = 0;
    SI(f_046) = 0;
    SI(ch1_coupling) = 0;
    SI(ch1_vdiv_index) = 6;
    SI(ch1_offset) = lvl;
    SI(ch2_enabled) = 0;
    SI(ch2_fft_enabled) = 0;
    SI(f_10a) = 0;
    SI(ch2_coupling) = 0;
    SI(ch2_vdiv_index) = 6;
    SI(ch2_offset) = lvl;
    SI(f_274) = 1;
    SI(f_275) = 1;
    SI(trigger_mode) = 0;
    SI(trigger_channel) = 0;
    SI(trigger_edge) = 0;
    SI(trigger_hf_mode) = 0;
    SI(f_64c) = 0;
    SI(f_70e) = 0;
    SI(f_54f) = 0;
    SI(f_54e) = 0;
    SI(waveform_light) = 0x32;
    SI(f_672) = 0x14;
    SI(f_656) = 0x1E;
    w8 = SI(plot_width);
    SI(f_673) = 2;
    SI(time_cursors_on) = 0;
    SI(f_3da530) = 0;
    SI(cursor_a) = (w8 * 0xAAABu) >> 17;
    SI(cursor_b) = (u32)(((long long)(int)(w8 * 2) * 0x55555556LL) >> 32);
    SI(f_02a) = 0;
    SI(acq_path_mode) = 0;
    SI(ch1_runtime_state) = 1;
    H16(SYS + 0x3A) = 0;
    SI(cursor_x1) = (w8 * 0xCCCDu) >> 18;
    wa = SI(plot_height);
    SI(f_710) = 0;
    SI(palette_alt) = 0;
    SI(grid_on) = 1;
    SI(hires_option) = 0;
    SI(cursor_y1) = wa >> 2;
    SI(f_650) = 1;
    SI(f_651) = 1;
    t = 3 * wa;
    SI(ch1_avg_count) = 0;
    SI(ch2_avg_count) = 0;
    SI(ch1_avg_limit) = 1;
    SI(ch2_avg_limit) = 1;
    SI(f_874) = 0;
    SI(f_580) = 1;
    SI(step_limiter_on) = 1;
    SI(f_586) = 0;
    H16(SYS + 0x664) = 0;
    SI(f_666) = 0;
    SI(fine_move) = 0;
    SI(f_652) = 0;
    SI(f_77a) = 1;
    SI(auto_off_minutes) = 0;
    SI(f_780) = 0;
    SI(f_784) = 0;
    SI(f_788) = 0;
    SI(cursor_x_on) = 0;
    SI(cursor_y_on) = 0;
    SI(cursor_x2) = (u32)(((long long)(int)(w8 * 4) * 0x66666667LL) >> 32) >> 1;
    SI(cursor_y2) = (t + ((t >> 1) >> 30)) >> 2;
    SI(f_804) = 0;
    SI(f_878) = 0;
    g_touch->enabled = 1;
    runtime_state_commit_force();
}
