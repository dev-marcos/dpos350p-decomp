/* SYS_INFO: the firmware's main state block at 0x40544E68 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level (HW-CONFIRMED / STATIC-CONFIRMED / DERIVED / OPEN) and
 * its source; a field without evidence keeps its offset as its name (f_XXX). The Hack names come
 * from third_party/FNIRSI-DPOS350P-Hack/docs/SYS_INFO.md.
 *
 * Use SI(field) to read or write a field: it is a volatile access at 0x40544E68 + offsetof(field),
 * which the compiler turns into the same base + offset instructions as the raw SYS8/16/32/64(off)
 * accessors, so the objects stay identical. Struct member syntax on a volatile struct pointer would
 * let GCC pick a different base register split, so it is not used. */
#ifndef SYS_INFO_H
#define SYS_INFO_H

#include <stddef.h>

#define SYS_INFO_BASE 0x40544E68u

struct sys_info {
    unsigned int f_000;                      /* +0x000 */
    unsigned short plot_x0;                  /* +0x004 DERIVED: u16 left edge of the plot area (0); read as x0 by the trace drawers */
    unsigned short plot_y0;                  /* +0x006 DERIVED: u16 top edge of the plot area (0x30) */
    unsigned short plot_width;               /* +0x008 DERIVED: u16 plot width (0x3B6 = 950); the view bounds clamp to 0..width. Hack calls it SYS_ACQ_TARGET */
    unsigned short plot_height;              /* +0x00A DERIVED: u16 plot height (0x1F7 = 503); trigger level clamps use it */
    unsigned short plot_x_mid;               /* +0x00C DERIVED: u16 horizontal centre of the plot (0x1DB = 475), added to the view offsets */
    unsigned short level_default;            /* +0x00E DERIVED: u16 default trigger / offset level (251) used by acq_defaults_reset */
    unsigned short window_lo;                /* +0x010 DERIVED: u16 low end of the first search window (window results lo / hi / hi - lo) */
    unsigned short window_hi;                /* +0x012 DERIVED: u16 high end of the first search window */
    unsigned short window_span;              /* +0x014 DERIVED: u16 hi - lo of the window; also the gate of the per-trace window routine */
    unsigned short f_016;                    /* +0x016 */
    unsigned short f_018;                    /* +0x018 */
    unsigned short f_01a;                    /* +0x01A */
    unsigned short f_01c;                    /* +0x01C */
    unsigned short f_01e;                    /* +0x01E */
    unsigned short grid_step;                /* +0x020 DERIVED: u16 pixels between grid lines (grid on: SYS+0x655) */
    unsigned short f_022;                    /* +0x022 */
    unsigned short view_window_width;        /* +0x024 DERIVED: u16 width of the view window drawn over the record (C x width / step, >= 6) */
    unsigned short view_window_centre;       /* +0x026 DERIVED: u16 centre of the view window: (position_ref - view_position) / step + width / 2 */
    unsigned char acq_path_mode;             /* +0x028 STATIC-CONFIRMED: Hack SYS_INFO.md */
    unsigned char acq_path_mode_shadow;      /* +0x029 STATIC-CONFIRMED: Hack SYS_INFO.md */
    unsigned char f_02a;                     /* +0x02A */
    unsigned char _pad0[0x5];
    unsigned long long f_030;                /* +0x030 */
    unsigned char acq_done;                  /* +0x038 HW-CONFIRMED: Hack SYS_INFO.md; bit 0 of FPGA selector 0x0A */
    unsigned char _pad1[0x1];
    unsigned char fpga_cmd62_value;          /* +0x03A STATIC-CONFIRMED: Hack SYS_INFO.md (semantics unknown) */
    unsigned char fpga_cmd61_value;          /* +0x03B STATIC-CONFIRMED: Hack SYS_INFO.md (semantics unknown) */
    unsigned char acq_transition_flag;       /* +0x03C STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned char f_03d;                     /* +0x03D */
    unsigned char ch1_enabled;               /* +0x03E HW-CONFIRMED: Hack SYS_INFO.md */
    unsigned char ch1_coupling;              /* +0x03F STATIC-CONFIRMED: Hack SYS_INFO.md; menu setting */
    unsigned char ch1_probe;                 /* +0x040 DERIVED: probe attenuation 1X / 10X / 100X (menu entry 27) */
    unsigned char ch1_vdiv_index;            /* +0x041 DERIVED: volts/div index, stepped by the auto-range code */
    unsigned char ch1_fft_enabled;           /* +0x042 DERIVED: fft_hann_spectrum windows CH1 only when this and ch1_enabled are set */
    unsigned char _pad2[0x1];
    unsigned short ch1_fpga_2a;              /* +0x044 DERIVED: CH1 value of the 16-bit FPGA selector 0x2A (self-test steps it from 3000) */
    unsigned char f_046;                     /* +0x046 */
    unsigned char ch1_bandwidth_filter;      /* +0x047 STATIC-CONFIRMED: Hack SYS_INFO.md; menu setting */
    unsigned char f_048;                     /* +0x048 */
    unsigned char _pad3[0x7];
    unsigned long long ch1_scale;            /* +0x050 DERIVED: s64 scale of CH1 (divisor of the offset and position maths) */
    long long f_058;                         /* +0x058 */
    unsigned int ch1_offset;                 /* +0x060 DERIVED: CH1 vertical offset word */
    unsigned int f_064;                      /* +0x064 */
    unsigned long long ch1_bw_limit;         /* +0x068 DERIVED: s64 CH1 bandwidth limit, from the coupling byte */
    unsigned char _pad4[0x68];
    unsigned long long f_0d8;                /* +0x0D8 */
    unsigned int f_0e0;                      /* +0x0E0 */
    unsigned int ch1_scale_q10;              /* +0x0E4 DERIVED: CH1 scale x 1024: (s64 +0x58 << 10) / +0x50; start of channel set 0 */
    unsigned int f_0e8;                      /* +0x0E8 */
    unsigned char _pad5[0xC];
    unsigned long long f_0f8;                /* +0x0F8 */
    unsigned char bank_mode;                 /* +0x100 DERIVED: channel / bank mode (FPGA selector 0x2E, 50 ms settle); picks the CSI bank capture */
    unsigned char ch1_runtime_state;         /* +0x101 STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned char ch2_enabled;               /* +0x102 HW-CONFIRMED: Hack SYS_INFO.md */
    unsigned char ch2_coupling;              /* +0x103 STATIC-CONFIRMED: Hack SYS_INFO.md; menu setting */
    unsigned char ch2_probe;                 /* +0x104 DERIVED: probe attenuation of CH2 */
    unsigned char ch2_vdiv_index;            /* +0x105 DERIVED: volts/div index of CH2 */
    unsigned char ch2_fft_enabled;           /* +0x106 DERIVED: as ch1_fft_enabled */
    unsigned char _pad6[0x1];
    unsigned short ch2_fpga_2d;              /* +0x108 DERIVED: CH2 value written with fpga_write_sel_0x2d */
    unsigned char f_10a;                     /* +0x10A */
    unsigned char ch2_bandwidth_filter;      /* +0x10B STATIC-CONFIRMED: Hack SYS_INFO.md; menu entry 32 (Full 350M / 150M / 20M) */
    unsigned char f_10c;                     /* +0x10C */
    unsigned char _pad7[0x3];
    unsigned long long ch2_scale;            /* +0x110 DERIVED: s64 scale of CH2 */
    long long f_118;                         /* +0x118 */
    unsigned int ch2_offset;                 /* +0x120 DERIVED: CH2 vertical offset word */
    unsigned int f_124;                      /* +0x124 */
    unsigned long long f_128;                /* +0x128 */
    unsigned char _pad8[0x68];
    unsigned int f_198;                      /* +0x198 */
    unsigned int f_19c;                      /* +0x19C */
    unsigned int f_1a0;                      /* +0x1A0 */
    unsigned int ch2_scale_q10;              /* +0x1A4 DERIVED: CH2 scale x 1024; start of channel set 1 */
    unsigned int f_1a8;                      /* +0x1A8 */
    unsigned char f_1ac;                     /* +0x1AC */
    unsigned char _pad9[0x1];
    unsigned short f_1ae;                    /* +0x1AE */
    unsigned char sample_divisor;            /* +0x1B0 DERIVED: sample divisor 1 or 2 from sample_divisor_setup; also picks the trace layout */
    unsigned char repeat_factor;             /* +0x1B1 DERIVED: decimation / repeat factor of the time base */
    unsigned char f_1b2;                     /* +0x1B2 */
    unsigned char _pad10[0x1];
    unsigned short f_1b4;                    /* +0x1B4 */
    unsigned short f_1b6;                    /* +0x1B6 */
    unsigned char f_1b8;                     /* +0x1B8 */
    unsigned char f_1b9;                     /* +0x1B9 */
    unsigned char f_1ba;                     /* +0x1BA */
    unsigned char _pad11[0x1];
    unsigned int sample_shift;               /* +0x1BC DERIVED: right shift applied to each sample before it is drawn */
    unsigned int acq_geometry_1c0;           /* +0x1C0 STATIC-CONFIRMED: Hack SYS_INFO.md (strongly inferred) */
    unsigned int ch1_gain_table;             /* +0x1C4 DERIVED: CH1 gain table, indexed by the range byte SYS+0x41 */
    unsigned int f_1c8;                      /* +0x1C8 */
    unsigned int f_1cc;                      /* +0x1CC */
    unsigned int f_1d0;                      /* +0x1D0 */
    unsigned int f_1d4;                      /* +0x1D4 */
    unsigned int f_1d8;                      /* +0x1D8 */
    unsigned int f_1dc;                      /* +0x1DC */
    unsigned int f_1e0;                      /* +0x1E0 */
    unsigned int f_1e4;                      /* +0x1E4 */
    unsigned int f_1e8;                      /* +0x1E8 */
    unsigned int f_1ec;                      /* +0x1EC */
    unsigned int f_1f0;                      /* +0x1F0 */
    unsigned int f_1f4;                      /* +0x1F4 */
    unsigned char _pad12[0x1C];
    unsigned int ch2_gain_table;             /* +0x214 DERIVED: CH2 gain table, indexed by the range byte SYS+0x105 */
    unsigned int f_218;                      /* +0x218 */
    unsigned int f_21c;                      /* +0x21C */
    unsigned int f_220;                      /* +0x220 */
    unsigned int f_224;                      /* +0x224 */
    unsigned int f_228;                      /* +0x228 */
    unsigned int f_22c;                      /* +0x22C */
    unsigned int f_230;                      /* +0x230 */
    unsigned int f_234;                      /* +0x234 */
    unsigned int f_238;                      /* +0x238 */
    unsigned int f_23c;                      /* +0x23C */
    unsigned int f_240;                      /* +0x240 */
    unsigned int f_244;                      /* +0x244 */
    unsigned char _pad13[0x1C];
    unsigned int raw_shift;                  /* +0x264 DERIVED: right shift of (q x raw + off) in the sample conversion */
    unsigned int acq_geometry_268;           /* +0x268 STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned int acq_geometry_26c;           /* +0x26C STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned int acq_geometry_270;           /* +0x270 STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned char f_274;                     /* +0x274 */
    unsigned char f_275;                     /* +0x275 */
    unsigned short f_276;                    /* +0x276 */
    unsigned char _pad14[0x1];
    unsigned char f_279;                     /* +0x279 */
    unsigned char trigger_mode;              /* +0x27A STATIC-CONFIRMED: Hack SYS_INFO.md; menu setting */
    unsigned char trigger_edge;              /* +0x27B STATIC-CONFIRMED: Hack SYS_INFO.md; menu setting */
    unsigned char trigger_channel;           /* +0x27C STATIC-CONFIRMED: Hack SYS_INFO.md; menu setting */
    unsigned char _pad15[0x1];
    unsigned short trigger_level;            /* +0x27E DERIVED: trigger level row, clamped to plot_height margins */
    unsigned short f_280;                    /* +0x280 */
    unsigned short trigger_multiplier;       /* +0x282 DERIVED: trigger multiplier 1 / 2 / 3 / 6, from trigger_hf_mode */
    unsigned char trigger_hf_mode;           /* +0x284 DERIVED: trigger HF1..HF3 option 0..3 (drawn in the 'TRI' toolbar item) */
    unsigned char _pad16[0x3];
    unsigned int trigger_offset_1b;          /* +0x288 DERIVED: trigger offset sent with FPGA selector 0x1B: (record / 2 + k) mod L */
    unsigned int trigger_offset_1c;          /* +0x28C DERIVED: trigger offset sent with FPGA selector 0x1C: (record - a) mod L */
    unsigned int fpga_position;              /* +0x290 DERIVED: position from the 13-bit FPGA value of selector 0x14 (fpga_position_update) */
    unsigned int f_294;                      /* +0x294 */
    unsigned int f_298;                      /* +0x298 */
    unsigned short f_29c;                    /* +0x29C */
    unsigned short f_29e;                    /* +0x29E */
    unsigned int f_2a0;                      /* +0x2A0 */
    unsigned char f_2a4;                     /* +0x2A4 */
    unsigned char f_2a5;                     /* +0x2A5 */
    unsigned char f_2a6;                     /* +0x2A6 */
    unsigned char timebase_index_max;        /* +0x2A7 DERIVED: upper clamp of the time-base index */
    unsigned char timebase_index_min;        /* +0x2A8 DERIVED: lower clamp of the time-base index */
    unsigned char slow_timebase_index;       /* +0x2A9 DERIVED: first time-base index of the slow (single) mode: SYS+0x28 = index >= this */
    unsigned char zoom_on;                   /* +0x2AA DERIVED: zoom view flag (divides the trace heights by 3) */
    unsigned char _pad17[0x1];
    unsigned short f_2ac;                    /* +0x2AC */
    unsigned char _pad18[0x2];
    unsigned int block_points;               /* +0x2B0 DERIVED: points per display block (record J holds this many halfwords per channel) */
    unsigned int f_2b4;                      /* +0x2B4 */
    unsigned int f_2b8;                      /* +0x2B8 */
    unsigned int f_2bc;                      /* +0x2BC */
    unsigned int f_2c0;                      /* +0x2C0 */
    unsigned int block_start;                /* +0x2C4 DERIVED: per-block start indices into the capture buffer */
    unsigned char _pad19[0xC4];
    unsigned int block_base;                 /* +0x38C DERIVED: per-block base indices */
    unsigned char _pad20[0xC4];
    unsigned int block_len;                  /* +0x454 DERIVED: per-block sample counts copied into the display buffer */
    unsigned char _pad21[0xC4];
    unsigned char f_51c;                     /* +0x51C */
    unsigned char f_51d;                     /* +0x51D */
    unsigned char _pad22[0x2];
    unsigned int record_length;              /* +0x520 DERIVED: record length L (single mode uses it as the sample count) */
    unsigned int sample_rate;                /* +0x524 DERIVED: sample rate; SYS+0x1B1 = this / divisor */
    unsigned long long f_528;                /* +0x528 */
    unsigned long long f_530;                /* +0x530 */
    unsigned long long f_538;                /* +0x538 */
    unsigned int f_540;                      /* +0x540 */
    unsigned short f_544;                    /* +0x544 */
    unsigned char _pad23[0x4];
    unsigned short f_54a;                    /* +0x54A */
    unsigned char _pad24[0x2];
    unsigned char f_54e;                     /* +0x54E */
    unsigned char f_54f;                     /* +0x54F */
    unsigned short capture_count;            /* +0x550 HW-CONFIRMED: Hack SYS_INFO.md */
    unsigned char _pad25[0x2];
    unsigned int f_554;                      /* +0x554 */
    unsigned int bank_count;                 /* +0x558 DERIVED: number of interleaved sample banks */
    unsigned int f_55c;                      /* +0x55C */
    unsigned int f_560;                      /* +0x560 */
    unsigned int f_564;                      /* +0x564 */
    unsigned int f_568;                      /* +0x568 */
    unsigned int f_56c;                      /* +0x56C */
    long long position_ref;                  /* +0x570 DERIVED: s64 position reference (timebase setters) */
    long long view_position;                 /* +0x578 DERIVED: s64 view position, clamped around position_ref */
    unsigned char f_580;                     /* +0x580 */
    unsigned char _pad26[0x1];
    unsigned short f_582;                    /* +0x582 */
    unsigned short f_584;                    /* +0x584 */
    unsigned short f_586;                    /* +0x586 */
    unsigned char timebase_index;            /* +0x588 DERIVED: time-base index (11 = 100 MHz clock); Hack: SYS_ACQ_PROFILE_OR_MODE */
    unsigned char _pad27[0x7];
    unsigned long long timebase_period;      /* +0x590 DERIVED: s64 time per pixel: entry timebase_index of the step table SYS+0xBD0 */
    unsigned char zoom_index;                /* +0x598 DERIVED: zoom time-base index byte (zoom_index_fit steps it) */
    unsigned char _pad28[0x7];
    long long zoom_period;                   /* +0x5A0 DERIVED: s64 zoom time per pixel: entry zoom_index of the step table */
    unsigned long long f_5a8;                /* +0x5A8 */
    unsigned char committed_timebase_index;  /* +0x5B0 DERIVED: time-base index committed by runtime_state_commit_copy */
    unsigned char _pad29[0x7];
    unsigned long long f_5b8;                /* +0x5B8 */
    unsigned char _pad30[0x10];
    unsigned short f_5d0;                    /* +0x5D0 */
    unsigned char _pad31[0x2];
    unsigned int f_5d4;                      /* +0x5D4 */
    unsigned int f_5d8;                      /* +0x5D8 */
    unsigned char _pad32[0x4];
    long long f_5e0;                         /* +0x5E0 */
    unsigned int f_5e8;                      /* +0x5E8 */
    unsigned int f_5ec;                      /* +0x5EC */
    unsigned int sample_clock_hz;            /* +0x5F0 DERIVED: MS5351 clock last programmed; reprogrammed when it differs */
    unsigned short segment_count;            /* +0x5F4 DERIVED: u16 segment count (repeat): SYS+0x5D0, or 1 in the single modes */
    unsigned char _pad33[0x2];
    unsigned int samples_per_column;         /* +0x5F8 DERIVED: samples averaged into one plot column */
    unsigned int sample_limit;               /* +0x5FC DERIVED: sample count limit of the main view */
    unsigned int view_scale;                 /* +0x600 DERIVED: scale of the main view (smallest j making sample_limit x j a multiple) */
    unsigned int view_step;                  /* +0x604 DERIVED: step of the main view */
    unsigned int view_offset_left;           /* +0x608 DERIVED: left offset word of the main view */
    unsigned int f_60c;                      /* +0x60C */
    unsigned int zoom_samples_per_column;    /* +0x610 DERIVED: samples averaged into one zoom column */
    unsigned int zoom_sample_limit;          /* +0x614 DERIVED: sample count limit of the zoom view */
    unsigned int zoom_scale;                 /* +0x618 DERIVED: scale of the zoom view */
    unsigned int zoom_step;                  /* +0x61C DERIVED: step of the zoom view */
    unsigned int zoom_offset_left;           /* +0x620 DERIVED: left offset word of the zoom view (view_bounds_update) */
    unsigned int zoom_offset_right;          /* +0x624 DERIVED: right offset word of the zoom view */
    unsigned char f_628;                     /* +0x628 */
    unsigned char _pad34[0x3];
    unsigned int acq_geometry_62c;           /* +0x62C STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned short f_630;                    /* +0x630 */
    unsigned short f_632;                    /* +0x632 */
    unsigned char _pad35[0x8];
    unsigned int runtime_quotient;           /* +0x63C STATIC-CONFIRMED: Hack SYS_INFO.md */
    unsigned short f_640;                    /* +0x640 */
    unsigned short f_642;                    /* +0x642 */
    unsigned short f_644;                    /* +0x644 */
    unsigned char _pad36[0x6];
    unsigned int f_64c;                      /* +0x64C */
    unsigned char f_650;                     /* +0x650 */
    unsigned char f_651;                     /* +0x651 */
    unsigned char f_652;                     /* +0x652 */
    unsigned char palette_alt;               /* +0x653 DERIVED: colour palette: 0x42E45864 when 0, else 0x42E47B8C */
    unsigned char waveform_light;            /* +0x654 DERIVED: 'Waveform light' percentage, set by the slider */
    unsigned char grid_on;                   /* +0x655 DERIVED: grid drawn (plot_grid_draw) */
    unsigned char f_656;                     /* +0x656 */
    unsigned char _pad37[0x1];
    unsigned int auto_off_minutes;           /* +0x658 DERIVED: auto power-off delay in minutes */
    unsigned int auto_off_deadline;          /* +0x65C DERIVED: tick of the next auto power-off */
    unsigned char hires_option;              /* +0x660 DERIVED: high-resolution option 0..8: 'OFF', '8 bit', '9 bit' .. */
    unsigned char _pad38[0x1];
    unsigned short moving_avg_width;         /* +0x662 DERIVED: u16 box filter width of channel_moving_average (off below 2) */
    unsigned char f_664;                     /* +0x664 */
    unsigned char f_665;                     /* +0x665 */
    unsigned short f_666;                    /* +0x666 */
    unsigned char _pad39[0x9];
    unsigned char fine_move;                 /* +0x671 DERIVED: touch drags move ten times slower (touch_drag_delta divides by 10) */
    unsigned char f_672;                     /* +0x672 */
    unsigned char f_673;                     /* +0x673 */
    unsigned char lane_masks;                /* +0x674 DERIVED: the 8 lane bytes smec_byte_params_sync sends */
    unsigned char f_675;                     /* +0x675 */
    unsigned char f_676;                     /* +0x676 */
    unsigned char f_677;                     /* +0x677 */
    unsigned char f_678;                     /* +0x678 */
    unsigned char f_679;                     /* +0x679 */
    unsigned char f_67a;                     /* +0x67A */
    unsigned char f_67b;                     /* +0x67B */
    unsigned char _pad40[0x58];
    unsigned int lane_phase;                 /* +0x6D4 DERIVED: per-lane phases, 8 bytes */
    unsigned int f_6d8;                      /* +0x6D8 */
    unsigned char lane_bank_map;             /* +0x6DC DERIVED: lane -> bank map, 8 bytes */
    unsigned char f_6dd;                     /* +0x6DD */
    unsigned char f_6de;                     /* +0x6DE */
    unsigned char f_6df;                     /* +0x6DF */
    unsigned char f_6e0;                     /* +0x6E0 */
    unsigned char f_6e1;                     /* +0x6E1 */
    unsigned char f_6e2;                     /* +0x6E2 */
    unsigned char f_6e3;                     /* +0x6E3 */
    unsigned char f_6e4;                     /* +0x6E4 */
    unsigned char f_6e5;                     /* +0x6E5 */
    unsigned char f_6e6;                     /* +0x6E6 */
    unsigned char f_6e7;                     /* +0x6E7 */
    unsigned int f_6e8;                      /* +0x6E8 */
    unsigned char f_6ec;                     /* +0x6EC */
    unsigned char f_6ed;                     /* +0x6ED */
    unsigned char runtime_ready;             /* +0x6EE HW-CONFIRMED: Hack SYS_INFO.md; bit 0 of FPGA selector 0x0B */
    unsigned char _pad41[0x1];
    unsigned short runtime_fpga_position;    /* +0x6F0 HW-CONFIRMED: Hack SYS_INFO.md */
    unsigned short runtime_position_base;    /* +0x6F2 STATIC-CONFIRMED: Hack SYS_INFO.md (working name) */
    unsigned short runtime_field_6f4;        /* +0x6F4 OPEN: Hack SYS_INFO.md (unknown) */
    unsigned short runtime_step;             /* +0x6F6 STATIC-CONFIRMED: Hack SYS_INFO.md (strongly inferred; also called copy length) */
    unsigned short runtime_coarse_step;      /* +0x6F8 STATIC-CONFIRMED: Hack SYS_INFO.md (strongly inferred) */
    unsigned char _pad42[0x2];
    unsigned int runtime_fine_progress;      /* +0x6FC HW-CONFIRMED: Hack SYS_INFO.md */
    unsigned int runtime_coarse_progress;    /* +0x700 HW-CONFIRMED: Hack SYS_INFO.md */
    unsigned int runtime_field_704;          /* +0x704 OPEN: Hack SYS_INFO.md (unknown); start sample of the display copy */
    unsigned int runtime_progress_threshold; /* +0x708 STATIC-CONFIRMED: Hack SYS_INFO.md (strongly inferred) */
    unsigned char _pad43[0x1];
    unsigned char step_limiter_on;           /* +0x70D DERIVED: enables channel_step_limiter */
    unsigned char f_70e;                     /* +0x70E */
    unsigned char _pad44[0x1];
    unsigned int f_710;                      /* +0x710 */
    unsigned char _pad45[0x40];
    unsigned short f_754;                    /* +0x754 */
    unsigned short f_756;                    /* +0x756 */
    unsigned short capture_slot_count;       /* +0x758 DERIVED: u16 number of slots of the capture list (list C) */
    unsigned char _pad46[0x2];
    unsigned int rec_h_start;                /* +0x75C DERIVED: settings record H: start in the settings block */
    unsigned int rec_i_start;                /* +0x760 DERIVED: settings record I: start */
    unsigned int rec_j_start;                /* +0x764 DERIVED: settings record J: start */
    unsigned int rec_h_size;                 /* +0x768 DERIVED: settings record H: size */
    unsigned int rec_i_size;                 /* +0x76C DERIVED: settings record I: size */
    unsigned int rec_j_size;                 /* +0x770 DERIVED: settings record J: size */
    unsigned int rec_hij_end;                /* +0x774 DERIVED: end of records H..J */
    unsigned char battery_level;             /* +0x778 DERIVED: battery icon fill (2 pixels per step) */
    unsigned char charging;                  /* +0x779 STATIC-CONFIRMED: Hack SYS_INFO.md */
    unsigned char f_77a;                     /* +0x77A */
    unsigned char _pad47[0x1];
    unsigned int f_77c;                      /* +0x77C */
    unsigned int f_780;                      /* +0x780 */
    unsigned int f_784;                      /* +0x784 */
    unsigned int f_788;                      /* +0x788 */
    unsigned char _pad48[0x20];
    unsigned char menu_language;             /* +0x7AC DERIVED: index of the menu string translations */
    unsigned char _pad49[0x3];
    unsigned int intensity_divisor;          /* +0x7B0 DERIVED: divisor trace_intensity_draw uses (max / 75) */
    unsigned int ch1_buffer;                 /* +0x7B4 DERIVED: address of the CH1 sample buffer */
    unsigned int ch2_buffer;                 /* +0x7B8 DERIVED: address of the CH2 sample buffer */
    unsigned int capture_samples;            /* +0x7BC DERIVED: u16 sample count of the capture buffer */
    unsigned char settings_slot;             /* +0x7C0 DERIVED: settings slot number 1..5 (settings_slot_buttons) */
    unsigned char _pad50[0x3];
    unsigned int rec_f_flash_addr;           /* +0x7C4 DERIVED: SPI flash address of settings record F */
    unsigned int f_7c8;                      /* +0x7C8 */
    unsigned int f_7cc;                      /* +0x7CC */
    unsigned int f_7d0;                      /* +0x7D0 */
    unsigned int f_7d4;                      /* +0x7D4 */
    unsigned int f_7d8;                      /* +0x7D8 */
    unsigned int rec_e_flash_addr;           /* +0x7DC DERIVED: SPI flash address of settings record E */
    unsigned int calibration_flash_addr;     /* +0x7E0 HW-CONFIRMED: Hack SYS_INFO.md (SYS_CALIBRATION_STORAGE_ADDRESS) */
    unsigned int rec_b_flash_addr;           /* +0x7E4 DERIVED: SPI flash address of settings record B */
    unsigned int rec_g_flash_addr;           /* +0x7E8 DERIVED: SPI flash address of settings record G */
    unsigned int rec_d_flash_addr;           /* +0x7EC DERIVED: SPI flash address of settings record D */
    unsigned int rec_c_flash_addr;           /* +0x7F0 DERIVED: SPI flash address of settings record C */
    unsigned int rec_a_flash_addr;           /* +0x7F4 DERIVED: SPI flash address of settings record A */
    unsigned int fft_size;                   /* +0x7F8 DERIVED: FFT length (0x800) */
    unsigned char fpga_probe_result;         /* +0x7FC DERIVED: FPGA identification probe result (A5 A5 5A 5A = OK) */
    unsigned char _pad51[0x3];
    unsigned char f_800;                     /* +0x800 */
    unsigned char f_801;                     /* +0x801 */
    unsigned char cursor_x_on;               /* +0x802 DERIVED: vertical (time) cursors shown */
    unsigned char cursor_y_on;               /* +0x803 DERIVED: horizontal (level) cursors shown */
    unsigned char f_804;                     /* +0x804 */
    unsigned char _pad52[0x3];
    unsigned int cursor_x1;                  /* +0x808 DERIVED: time cursor 1 position */
    unsigned int cursor_x2;                  /* +0x80C DERIVED: time cursor 2 position */
    unsigned int cursor_y1;                  /* +0x810 DERIVED: level cursor 1 position */
    unsigned int cursor_y2;                  /* +0x814 DERIVED: level cursor 2 position */
    unsigned char _pad53[0x10];
    unsigned long long f_828;                /* +0x828 */
    unsigned long long f_830;                /* +0x830 */
    unsigned char _pad54[0x30];
    unsigned short f_868;                    /* +0x868 */
    unsigned short ch1_avg_count;            /* +0x86A DERIVED: CH1 averaging counter */
    unsigned short ch2_avg_count;            /* +0x86C DERIVED: CH2 averaging counter */
    unsigned short ch1_avg_limit;            /* +0x86E DERIVED: CH1 averaging limit */
    unsigned short ch2_avg_limit;            /* +0x870 DERIVED: CH2 averaging limit */
    unsigned char ch1_avg_ready;             /* +0x872 DERIVED: CH1 "average ready" byte */
    unsigned char ch2_avg_ready;             /* +0x873 DERIVED: CH2 "average ready" byte */
    unsigned int f_874;                      /* +0x874 */
    unsigned int f_878;                      /* +0x878 */
    unsigned int f_87c;                      /* +0x87C */
    unsigned char _pad55[0x64];
    unsigned char f_8e4;                     /* +0x8E4 */
    unsigned char _pad56[0x13];
    long long f_8f8;                         /* +0x8F8 */
    unsigned char _pad57[0x10];
    long long f_910;                         /* +0x910 */
    unsigned char _pad58[0x40];
    long long f_958;                         /* +0x958 */
    unsigned char _pad59[0x10];
    long long f_970;                         /* +0x970 */
    unsigned char _pad60[0x1B8];
    unsigned int ch1_amplitude;              /* +0xB30 DERIVED: CH1 measured amplitude word */
    unsigned int ch1_mean_pos;               /* +0xB34 DERIVED: CH1 measured mean position */
    unsigned char _pad61[0x14];
    unsigned int ch2_amplitude;              /* +0xB4C DERIVED: CH2 measured amplitude word */
    unsigned int ch2_mean_pos;               /* +0xB50 DERIVED: CH2 measured mean position */
    unsigned char _pad62[0xC];
    unsigned int ch1_top_ref;                /* +0xB60 DERIVED: CH1 top reference of the peak normalisation */
    unsigned int ch1_bottom_ref;             /* +0xB64 DERIVED: CH1 bottom reference of the peak normalisation */
    unsigned int f_b68;                      /* +0xB68 */
    unsigned int ch1_mid_level;              /* +0xB6C DERIVED: CH1 middle level m of the peak normalisation */
    unsigned char _pad63[0x8];
    unsigned int f_b78;                      /* +0xB78 */
    unsigned int f_b7c;                      /* +0xB7C */
    unsigned char _pad64[0x8];
    unsigned int f_b88;                      /* +0xB88 */
    unsigned int f_b8c;                      /* +0xB8C */
    unsigned int f_b90;                      /* +0xB90 */
    unsigned int f_b94;                      /* +0xB94 */
    unsigned char _pad65[0x8];
    unsigned int f_ba0;                      /* +0xBA0 */
    unsigned int f_ba4;                      /* +0xBA4 */
    unsigned char _pad66[0x8];
    unsigned char outputs_applied;           /* +0xBB0 DERIVED: copy of the output settings last sent by outputs_apply */
    unsigned char f_bb1;                     /* +0xBB1 */
    unsigned char f_bb2;                     /* +0xBB2 */
    unsigned char f_bb3;                     /* +0xBB3 */
    unsigned int f_bb4;                      /* +0xBB4 */
    unsigned int f_bb8;                      /* +0xBB8 */
    unsigned int f_bbc;                      /* +0xBBC */
    unsigned int f_bc0;                      /* +0xBC0 */
    unsigned int f_bc4;                      /* +0xBC4 */
    unsigned char f_bc8;                     /* +0xBC8 */
    unsigned char f_bc9;                     /* +0xBC9 */
    unsigned char f_bca;                     /* +0xBCA */
    unsigned char _pad67[0x5];
    long long timebase_steps;                /* +0xBD0 DERIVED: the 36 s64 entries of the 1-2-5 time-base step table */
    long long f_bd8;                         /* +0xBD8 */
    long long f_be0;                         /* +0xBE0 */
    long long f_be8;                         /* +0xBE8 */
    long long f_bf0;                         /* +0xBF0 */
    long long f_bf8;                         /* +0xBF8 */
    long long f_c00;                         /* +0xC00 */
    long long f_c08;                         /* +0xC08 */
    long long f_c10;                         /* +0xC10 */
    long long f_c18;                         /* +0xC18 */
    long long f_c20;                         /* +0xC20 */
    long long f_c28;                         /* +0xC28 */
    long long f_c30;                         /* +0xC30 */
    long long f_c38;                         /* +0xC38 */
    long long f_c40;                         /* +0xC40 */
    long long f_c48;                         /* +0xC48 */
    long long f_c50;                         /* +0xC50 */
    long long f_c58;                         /* +0xC58 */
    long long f_c60;                         /* +0xC60 */
    long long f_c68;                         /* +0xC68 */
    long long f_c70;                         /* +0xC70 */
    long long f_c78;                         /* +0xC78 */
    long long f_c80;                         /* +0xC80 */
    long long f_c88;                         /* +0xC88 */
    long long f_c90;                         /* +0xC90 */
    long long f_c98;                         /* +0xC98 */
    long long f_ca0;                         /* +0xCA0 */
    long long f_ca8;                         /* +0xCA8 */
    long long f_cb0;                         /* +0xCB0 */
    long long f_cb8;                         /* +0xCB8 */
    long long f_cc0;                         /* +0xCC0 */
    long long f_cc8;                         /* +0xCC8 */
    long long f_cd0;                         /* +0xCD0 */
    long long f_cd8;                         /* +0xCD8 */
    long long f_ce0;                         /* +0xCE0 */
    long long f_ce8;                         /* +0xCE8 */
    long long f_cf0;                         /* +0xCF0 */
    long long f_cf8;                         /* +0xCF8 */
    long long f_d00;                         /* +0xD00 */
    long long f_d08;                         /* +0xD08 */
    long long f_d10;                         /* +0xD10 */
    long long f_d18;                         /* +0xD18 */
    long long f_d20;                         /* +0xD20 */
    unsigned char _pad68[0x268];
    unsigned char list_mark_mode;            /* +0xF90 DERIVED: file list: mark (multi-select) mode */
    unsigned char _pad69[0x9];
    unsigned short list_page_size;           /* +0xF9A DERIVED: file list: entries per page */
    unsigned short f_f9c;                    /* +0xF9C */
    short list_page_index;                   /* +0xF9E DERIVED: file list: current page */
    short list_count;                        /* +0xFA0 DERIVED: file list: number of entries */
    short list_page_count;                   /* +0xFA2 DERIVED: file list: number of pages */
    short list_selected;                     /* +0xFA4 DERIVED: file list: selected entry */
    unsigned short f_fa6;                    /* +0xFA6 */
    unsigned char _pad70[0x7916];
    unsigned short list_marked_count;        /* +0x88BE DERIVED: file list: number of marked entries */
    unsigned char path_buf;                  /* +0x88C0 DERIVED: path buffer (0x4054D728) the file lists build their paths in */
    unsigned char path_name;                 /* +0x88C1 DERIVED: name part of the path buffer */
    unsigned char _pad71[0x7];
    unsigned char f_88c9;                    /* +0x88C9 */
    unsigned char _pad72[0x2];
    unsigned char f_88cc;                    /* +0x88CC */
    unsigned char _pad73[0x100DB7];
    unsigned short bmp_width;                /* +0x109684 DERIVED: record I / screenshot BMP width */
    unsigned short bmp_height;               /* +0x109686 DERIVED: record I / screenshot BMP height */
    unsigned char _pad74[0x2D0EA4];
    unsigned char time_cursors_on;           /* +0x3DA52C DERIVED: enable byte of the time cursors C1 / C2 */
    unsigned char _pad75[0x1];
    unsigned short list_current;             /* +0x3DA52E DERIVED: file list C: current entry */
    unsigned short f_3da530;                 /* +0x3DA530 */
    unsigned char _pad76[0x2];
    unsigned int cursor_a;                   /* +0x3DA534 DERIVED: first word of the cursor pair cursor_pair_clamp keeps apart */
    unsigned int cursor_b;                   /* +0x3DA538 DERIVED: second word of that cursor pair */
    unsigned int capture_len;                /* +0x3DA53C DERIVED: length word of the selected capture record */
    unsigned int cursor_gap;                 /* +0x3DA540 DERIVED: minimum distance between cursor_a and cursor_b */
    unsigned int capture_cells;              /* +0x3DA544 DERIVED: halfword cells per capture record */
    unsigned char _pad77[0x3004];
    unsigned int theme_pressed;              /* +0x3DD54C DERIVED: theme colour of a pressed button */
    unsigned int theme_background;           /* +0x3DD550 DERIVED: theme colour of a menu background (text at +4, pressed row at +8) */
    unsigned int theme_text;                 /* +0x3DD554 DERIVED: theme colour of menu text */
};

_Static_assert(offsetof(struct sys_info, f_000) == 0x0, "f_000");
_Static_assert(offsetof(struct sys_info, plot_x0) == 0x4, "plot_x0");
_Static_assert(offsetof(struct sys_info, plot_y0) == 0x6, "plot_y0");
_Static_assert(offsetof(struct sys_info, plot_width) == 0x8, "plot_width");
_Static_assert(offsetof(struct sys_info, plot_height) == 0xA, "plot_height");
_Static_assert(offsetof(struct sys_info, plot_x_mid) == 0xC, "plot_x_mid");
_Static_assert(offsetof(struct sys_info, level_default) == 0xE, "level_default");
_Static_assert(offsetof(struct sys_info, window_lo) == 0x10, "window_lo");
_Static_assert(offsetof(struct sys_info, window_hi) == 0x12, "window_hi");
_Static_assert(offsetof(struct sys_info, window_span) == 0x14, "window_span");
_Static_assert(offsetof(struct sys_info, f_016) == 0x16, "f_016");
_Static_assert(offsetof(struct sys_info, f_018) == 0x18, "f_018");
_Static_assert(offsetof(struct sys_info, f_01a) == 0x1A, "f_01a");
_Static_assert(offsetof(struct sys_info, f_01c) == 0x1C, "f_01c");
_Static_assert(offsetof(struct sys_info, f_01e) == 0x1E, "f_01e");
_Static_assert(offsetof(struct sys_info, grid_step) == 0x20, "grid_step");
_Static_assert(offsetof(struct sys_info, f_022) == 0x22, "f_022");
_Static_assert(offsetof(struct sys_info, view_window_width) == 0x24, "view_window_width");
_Static_assert(offsetof(struct sys_info, view_window_centre) == 0x26, "view_window_centre");
_Static_assert(offsetof(struct sys_info, acq_path_mode) == 0x28, "acq_path_mode");
_Static_assert(offsetof(struct sys_info, acq_path_mode_shadow) == 0x29, "acq_path_mode_shadow");
_Static_assert(offsetof(struct sys_info, f_02a) == 0x2A, "f_02a");
_Static_assert(offsetof(struct sys_info, f_030) == 0x30, "f_030");
_Static_assert(offsetof(struct sys_info, acq_done) == 0x38, "acq_done");
_Static_assert(offsetof(struct sys_info, fpga_cmd62_value) == 0x3A, "fpga_cmd62_value");
_Static_assert(offsetof(struct sys_info, fpga_cmd61_value) == 0x3B, "fpga_cmd61_value");
_Static_assert(offsetof(struct sys_info, acq_transition_flag) == 0x3C, "acq_transition_flag");
_Static_assert(offsetof(struct sys_info, f_03d) == 0x3D, "f_03d");
_Static_assert(offsetof(struct sys_info, ch1_enabled) == 0x3E, "ch1_enabled");
_Static_assert(offsetof(struct sys_info, ch1_coupling) == 0x3F, "ch1_coupling");
_Static_assert(offsetof(struct sys_info, ch1_probe) == 0x40, "ch1_probe");
_Static_assert(offsetof(struct sys_info, ch1_vdiv_index) == 0x41, "ch1_vdiv_index");
_Static_assert(offsetof(struct sys_info, ch1_fft_enabled) == 0x42, "ch1_fft_enabled");
_Static_assert(offsetof(struct sys_info, ch1_fpga_2a) == 0x44, "ch1_fpga_2a");
_Static_assert(offsetof(struct sys_info, f_046) == 0x46, "f_046");
_Static_assert(offsetof(struct sys_info, ch1_bandwidth_filter) == 0x47, "ch1_bandwidth_filter");
_Static_assert(offsetof(struct sys_info, f_048) == 0x48, "f_048");
_Static_assert(offsetof(struct sys_info, ch1_scale) == 0x50, "ch1_scale");
_Static_assert(offsetof(struct sys_info, f_058) == 0x58, "f_058");
_Static_assert(offsetof(struct sys_info, ch1_offset) == 0x60, "ch1_offset");
_Static_assert(offsetof(struct sys_info, f_064) == 0x64, "f_064");
_Static_assert(offsetof(struct sys_info, ch1_bw_limit) == 0x68, "ch1_bw_limit");
_Static_assert(offsetof(struct sys_info, f_0d8) == 0xD8, "f_0d8");
_Static_assert(offsetof(struct sys_info, f_0e0) == 0xE0, "f_0e0");
_Static_assert(offsetof(struct sys_info, ch1_scale_q10) == 0xE4, "ch1_scale_q10");
_Static_assert(offsetof(struct sys_info, f_0e8) == 0xE8, "f_0e8");
_Static_assert(offsetof(struct sys_info, f_0f8) == 0xF8, "f_0f8");
_Static_assert(offsetof(struct sys_info, bank_mode) == 0x100, "bank_mode");
_Static_assert(offsetof(struct sys_info, ch1_runtime_state) == 0x101, "ch1_runtime_state");
_Static_assert(offsetof(struct sys_info, ch2_enabled) == 0x102, "ch2_enabled");
_Static_assert(offsetof(struct sys_info, ch2_coupling) == 0x103, "ch2_coupling");
_Static_assert(offsetof(struct sys_info, ch2_probe) == 0x104, "ch2_probe");
_Static_assert(offsetof(struct sys_info, ch2_vdiv_index) == 0x105, "ch2_vdiv_index");
_Static_assert(offsetof(struct sys_info, ch2_fft_enabled) == 0x106, "ch2_fft_enabled");
_Static_assert(offsetof(struct sys_info, ch2_fpga_2d) == 0x108, "ch2_fpga_2d");
_Static_assert(offsetof(struct sys_info, f_10a) == 0x10A, "f_10a");
_Static_assert(offsetof(struct sys_info, ch2_bandwidth_filter) == 0x10B, "ch2_bandwidth_filter");
_Static_assert(offsetof(struct sys_info, f_10c) == 0x10C, "f_10c");
_Static_assert(offsetof(struct sys_info, ch2_scale) == 0x110, "ch2_scale");
_Static_assert(offsetof(struct sys_info, f_118) == 0x118, "f_118");
_Static_assert(offsetof(struct sys_info, ch2_offset) == 0x120, "ch2_offset");
_Static_assert(offsetof(struct sys_info, f_124) == 0x124, "f_124");
_Static_assert(offsetof(struct sys_info, f_128) == 0x128, "f_128");
_Static_assert(offsetof(struct sys_info, f_198) == 0x198, "f_198");
_Static_assert(offsetof(struct sys_info, f_19c) == 0x19C, "f_19c");
_Static_assert(offsetof(struct sys_info, f_1a0) == 0x1A0, "f_1a0");
_Static_assert(offsetof(struct sys_info, ch2_scale_q10) == 0x1A4, "ch2_scale_q10");
_Static_assert(offsetof(struct sys_info, f_1a8) == 0x1A8, "f_1a8");
_Static_assert(offsetof(struct sys_info, f_1ac) == 0x1AC, "f_1ac");
_Static_assert(offsetof(struct sys_info, f_1ae) == 0x1AE, "f_1ae");
_Static_assert(offsetof(struct sys_info, sample_divisor) == 0x1B0, "sample_divisor");
_Static_assert(offsetof(struct sys_info, repeat_factor) == 0x1B1, "repeat_factor");
_Static_assert(offsetof(struct sys_info, f_1b2) == 0x1B2, "f_1b2");
_Static_assert(offsetof(struct sys_info, f_1b4) == 0x1B4, "f_1b4");
_Static_assert(offsetof(struct sys_info, f_1b6) == 0x1B6, "f_1b6");
_Static_assert(offsetof(struct sys_info, f_1b8) == 0x1B8, "f_1b8");
_Static_assert(offsetof(struct sys_info, f_1b9) == 0x1B9, "f_1b9");
_Static_assert(offsetof(struct sys_info, f_1ba) == 0x1BA, "f_1ba");
_Static_assert(offsetof(struct sys_info, sample_shift) == 0x1BC, "sample_shift");
_Static_assert(offsetof(struct sys_info, acq_geometry_1c0) == 0x1C0, "acq_geometry_1c0");
_Static_assert(offsetof(struct sys_info, ch1_gain_table) == 0x1C4, "ch1_gain_table");
_Static_assert(offsetof(struct sys_info, f_1c8) == 0x1C8, "f_1c8");
_Static_assert(offsetof(struct sys_info, f_1cc) == 0x1CC, "f_1cc");
_Static_assert(offsetof(struct sys_info, f_1d0) == 0x1D0, "f_1d0");
_Static_assert(offsetof(struct sys_info, f_1d4) == 0x1D4, "f_1d4");
_Static_assert(offsetof(struct sys_info, f_1d8) == 0x1D8, "f_1d8");
_Static_assert(offsetof(struct sys_info, f_1dc) == 0x1DC, "f_1dc");
_Static_assert(offsetof(struct sys_info, f_1e0) == 0x1E0, "f_1e0");
_Static_assert(offsetof(struct sys_info, f_1e4) == 0x1E4, "f_1e4");
_Static_assert(offsetof(struct sys_info, f_1e8) == 0x1E8, "f_1e8");
_Static_assert(offsetof(struct sys_info, f_1ec) == 0x1EC, "f_1ec");
_Static_assert(offsetof(struct sys_info, f_1f0) == 0x1F0, "f_1f0");
_Static_assert(offsetof(struct sys_info, f_1f4) == 0x1F4, "f_1f4");
_Static_assert(offsetof(struct sys_info, ch2_gain_table) == 0x214, "ch2_gain_table");
_Static_assert(offsetof(struct sys_info, f_218) == 0x218, "f_218");
_Static_assert(offsetof(struct sys_info, f_21c) == 0x21C, "f_21c");
_Static_assert(offsetof(struct sys_info, f_220) == 0x220, "f_220");
_Static_assert(offsetof(struct sys_info, f_224) == 0x224, "f_224");
_Static_assert(offsetof(struct sys_info, f_228) == 0x228, "f_228");
_Static_assert(offsetof(struct sys_info, f_22c) == 0x22C, "f_22c");
_Static_assert(offsetof(struct sys_info, f_230) == 0x230, "f_230");
_Static_assert(offsetof(struct sys_info, f_234) == 0x234, "f_234");
_Static_assert(offsetof(struct sys_info, f_238) == 0x238, "f_238");
_Static_assert(offsetof(struct sys_info, f_23c) == 0x23C, "f_23c");
_Static_assert(offsetof(struct sys_info, f_240) == 0x240, "f_240");
_Static_assert(offsetof(struct sys_info, f_244) == 0x244, "f_244");
_Static_assert(offsetof(struct sys_info, raw_shift) == 0x264, "raw_shift");
_Static_assert(offsetof(struct sys_info, acq_geometry_268) == 0x268, "acq_geometry_268");
_Static_assert(offsetof(struct sys_info, acq_geometry_26c) == 0x26C, "acq_geometry_26c");
_Static_assert(offsetof(struct sys_info, acq_geometry_270) == 0x270, "acq_geometry_270");
_Static_assert(offsetof(struct sys_info, f_274) == 0x274, "f_274");
_Static_assert(offsetof(struct sys_info, f_275) == 0x275, "f_275");
_Static_assert(offsetof(struct sys_info, f_276) == 0x276, "f_276");
_Static_assert(offsetof(struct sys_info, f_279) == 0x279, "f_279");
_Static_assert(offsetof(struct sys_info, trigger_mode) == 0x27A, "trigger_mode");
_Static_assert(offsetof(struct sys_info, trigger_edge) == 0x27B, "trigger_edge");
_Static_assert(offsetof(struct sys_info, trigger_channel) == 0x27C, "trigger_channel");
_Static_assert(offsetof(struct sys_info, trigger_level) == 0x27E, "trigger_level");
_Static_assert(offsetof(struct sys_info, f_280) == 0x280, "f_280");
_Static_assert(offsetof(struct sys_info, trigger_multiplier) == 0x282, "trigger_multiplier");
_Static_assert(offsetof(struct sys_info, trigger_hf_mode) == 0x284, "trigger_hf_mode");
_Static_assert(offsetof(struct sys_info, trigger_offset_1b) == 0x288, "trigger_offset_1b");
_Static_assert(offsetof(struct sys_info, trigger_offset_1c) == 0x28C, "trigger_offset_1c");
_Static_assert(offsetof(struct sys_info, fpga_position) == 0x290, "fpga_position");
_Static_assert(offsetof(struct sys_info, f_294) == 0x294, "f_294");
_Static_assert(offsetof(struct sys_info, f_298) == 0x298, "f_298");
_Static_assert(offsetof(struct sys_info, f_29c) == 0x29C, "f_29c");
_Static_assert(offsetof(struct sys_info, f_29e) == 0x29E, "f_29e");
_Static_assert(offsetof(struct sys_info, f_2a0) == 0x2A0, "f_2a0");
_Static_assert(offsetof(struct sys_info, f_2a4) == 0x2A4, "f_2a4");
_Static_assert(offsetof(struct sys_info, f_2a5) == 0x2A5, "f_2a5");
_Static_assert(offsetof(struct sys_info, f_2a6) == 0x2A6, "f_2a6");
_Static_assert(offsetof(struct sys_info, timebase_index_max) == 0x2A7, "timebase_index_max");
_Static_assert(offsetof(struct sys_info, timebase_index_min) == 0x2A8, "timebase_index_min");
_Static_assert(offsetof(struct sys_info, slow_timebase_index) == 0x2A9, "slow_timebase_index");
_Static_assert(offsetof(struct sys_info, zoom_on) == 0x2AA, "zoom_on");
_Static_assert(offsetof(struct sys_info, f_2ac) == 0x2AC, "f_2ac");
_Static_assert(offsetof(struct sys_info, block_points) == 0x2B0, "block_points");
_Static_assert(offsetof(struct sys_info, f_2b4) == 0x2B4, "f_2b4");
_Static_assert(offsetof(struct sys_info, f_2b8) == 0x2B8, "f_2b8");
_Static_assert(offsetof(struct sys_info, f_2bc) == 0x2BC, "f_2bc");
_Static_assert(offsetof(struct sys_info, f_2c0) == 0x2C0, "f_2c0");
_Static_assert(offsetof(struct sys_info, block_start) == 0x2C4, "block_start");
_Static_assert(offsetof(struct sys_info, block_base) == 0x38C, "block_base");
_Static_assert(offsetof(struct sys_info, block_len) == 0x454, "block_len");
_Static_assert(offsetof(struct sys_info, f_51c) == 0x51C, "f_51c");
_Static_assert(offsetof(struct sys_info, f_51d) == 0x51D, "f_51d");
_Static_assert(offsetof(struct sys_info, record_length) == 0x520, "record_length");
_Static_assert(offsetof(struct sys_info, sample_rate) == 0x524, "sample_rate");
_Static_assert(offsetof(struct sys_info, f_528) == 0x528, "f_528");
_Static_assert(offsetof(struct sys_info, f_530) == 0x530, "f_530");
_Static_assert(offsetof(struct sys_info, f_538) == 0x538, "f_538");
_Static_assert(offsetof(struct sys_info, f_540) == 0x540, "f_540");
_Static_assert(offsetof(struct sys_info, f_544) == 0x544, "f_544");
_Static_assert(offsetof(struct sys_info, f_54a) == 0x54A, "f_54a");
_Static_assert(offsetof(struct sys_info, f_54e) == 0x54E, "f_54e");
_Static_assert(offsetof(struct sys_info, f_54f) == 0x54F, "f_54f");
_Static_assert(offsetof(struct sys_info, capture_count) == 0x550, "capture_count");
_Static_assert(offsetof(struct sys_info, f_554) == 0x554, "f_554");
_Static_assert(offsetof(struct sys_info, bank_count) == 0x558, "bank_count");
_Static_assert(offsetof(struct sys_info, f_55c) == 0x55C, "f_55c");
_Static_assert(offsetof(struct sys_info, f_560) == 0x560, "f_560");
_Static_assert(offsetof(struct sys_info, f_564) == 0x564, "f_564");
_Static_assert(offsetof(struct sys_info, f_568) == 0x568, "f_568");
_Static_assert(offsetof(struct sys_info, f_56c) == 0x56C, "f_56c");
_Static_assert(offsetof(struct sys_info, position_ref) == 0x570, "position_ref");
_Static_assert(offsetof(struct sys_info, view_position) == 0x578, "view_position");
_Static_assert(offsetof(struct sys_info, f_580) == 0x580, "f_580");
_Static_assert(offsetof(struct sys_info, f_582) == 0x582, "f_582");
_Static_assert(offsetof(struct sys_info, f_584) == 0x584, "f_584");
_Static_assert(offsetof(struct sys_info, f_586) == 0x586, "f_586");
_Static_assert(offsetof(struct sys_info, timebase_index) == 0x588, "timebase_index");
_Static_assert(offsetof(struct sys_info, timebase_period) == 0x590, "timebase_period");
_Static_assert(offsetof(struct sys_info, zoom_index) == 0x598, "zoom_index");
_Static_assert(offsetof(struct sys_info, zoom_period) == 0x5A0, "zoom_period");
_Static_assert(offsetof(struct sys_info, f_5a8) == 0x5A8, "f_5a8");
_Static_assert(offsetof(struct sys_info, committed_timebase_index) == 0x5B0, "committed_timebase_index");
_Static_assert(offsetof(struct sys_info, f_5b8) == 0x5B8, "f_5b8");
_Static_assert(offsetof(struct sys_info, f_5d0) == 0x5D0, "f_5d0");
_Static_assert(offsetof(struct sys_info, f_5d4) == 0x5D4, "f_5d4");
_Static_assert(offsetof(struct sys_info, f_5d8) == 0x5D8, "f_5d8");
_Static_assert(offsetof(struct sys_info, f_5e0) == 0x5E0, "f_5e0");
_Static_assert(offsetof(struct sys_info, f_5e8) == 0x5E8, "f_5e8");
_Static_assert(offsetof(struct sys_info, f_5ec) == 0x5EC, "f_5ec");
_Static_assert(offsetof(struct sys_info, sample_clock_hz) == 0x5F0, "sample_clock_hz");
_Static_assert(offsetof(struct sys_info, segment_count) == 0x5F4, "segment_count");
_Static_assert(offsetof(struct sys_info, samples_per_column) == 0x5F8, "samples_per_column");
_Static_assert(offsetof(struct sys_info, sample_limit) == 0x5FC, "sample_limit");
_Static_assert(offsetof(struct sys_info, view_scale) == 0x600, "view_scale");
_Static_assert(offsetof(struct sys_info, view_step) == 0x604, "view_step");
_Static_assert(offsetof(struct sys_info, view_offset_left) == 0x608, "view_offset_left");
_Static_assert(offsetof(struct sys_info, f_60c) == 0x60C, "f_60c");
_Static_assert(offsetof(struct sys_info, zoom_samples_per_column) == 0x610, "zoom_samples_per_column");
_Static_assert(offsetof(struct sys_info, zoom_sample_limit) == 0x614, "zoom_sample_limit");
_Static_assert(offsetof(struct sys_info, zoom_scale) == 0x618, "zoom_scale");
_Static_assert(offsetof(struct sys_info, zoom_step) == 0x61C, "zoom_step");
_Static_assert(offsetof(struct sys_info, zoom_offset_left) == 0x620, "zoom_offset_left");
_Static_assert(offsetof(struct sys_info, zoom_offset_right) == 0x624, "zoom_offset_right");
_Static_assert(offsetof(struct sys_info, f_628) == 0x628, "f_628");
_Static_assert(offsetof(struct sys_info, acq_geometry_62c) == 0x62C, "acq_geometry_62c");
_Static_assert(offsetof(struct sys_info, f_630) == 0x630, "f_630");
_Static_assert(offsetof(struct sys_info, f_632) == 0x632, "f_632");
_Static_assert(offsetof(struct sys_info, runtime_quotient) == 0x63C, "runtime_quotient");
_Static_assert(offsetof(struct sys_info, f_640) == 0x640, "f_640");
_Static_assert(offsetof(struct sys_info, f_642) == 0x642, "f_642");
_Static_assert(offsetof(struct sys_info, f_644) == 0x644, "f_644");
_Static_assert(offsetof(struct sys_info, f_64c) == 0x64C, "f_64c");
_Static_assert(offsetof(struct sys_info, f_650) == 0x650, "f_650");
_Static_assert(offsetof(struct sys_info, f_651) == 0x651, "f_651");
_Static_assert(offsetof(struct sys_info, f_652) == 0x652, "f_652");
_Static_assert(offsetof(struct sys_info, palette_alt) == 0x653, "palette_alt");
_Static_assert(offsetof(struct sys_info, waveform_light) == 0x654, "waveform_light");
_Static_assert(offsetof(struct sys_info, grid_on) == 0x655, "grid_on");
_Static_assert(offsetof(struct sys_info, f_656) == 0x656, "f_656");
_Static_assert(offsetof(struct sys_info, auto_off_minutes) == 0x658, "auto_off_minutes");
_Static_assert(offsetof(struct sys_info, auto_off_deadline) == 0x65C, "auto_off_deadline");
_Static_assert(offsetof(struct sys_info, hires_option) == 0x660, "hires_option");
_Static_assert(offsetof(struct sys_info, moving_avg_width) == 0x662, "moving_avg_width");
_Static_assert(offsetof(struct sys_info, f_664) == 0x664, "f_664");
_Static_assert(offsetof(struct sys_info, f_665) == 0x665, "f_665");
_Static_assert(offsetof(struct sys_info, f_666) == 0x666, "f_666");
_Static_assert(offsetof(struct sys_info, fine_move) == 0x671, "fine_move");
_Static_assert(offsetof(struct sys_info, f_672) == 0x672, "f_672");
_Static_assert(offsetof(struct sys_info, f_673) == 0x673, "f_673");
_Static_assert(offsetof(struct sys_info, lane_masks) == 0x674, "lane_masks");
_Static_assert(offsetof(struct sys_info, f_675) == 0x675, "f_675");
_Static_assert(offsetof(struct sys_info, f_676) == 0x676, "f_676");
_Static_assert(offsetof(struct sys_info, f_677) == 0x677, "f_677");
_Static_assert(offsetof(struct sys_info, f_678) == 0x678, "f_678");
_Static_assert(offsetof(struct sys_info, f_679) == 0x679, "f_679");
_Static_assert(offsetof(struct sys_info, f_67a) == 0x67A, "f_67a");
_Static_assert(offsetof(struct sys_info, f_67b) == 0x67B, "f_67b");
_Static_assert(offsetof(struct sys_info, lane_phase) == 0x6D4, "lane_phase");
_Static_assert(offsetof(struct sys_info, f_6d8) == 0x6D8, "f_6d8");
_Static_assert(offsetof(struct sys_info, lane_bank_map) == 0x6DC, "lane_bank_map");
_Static_assert(offsetof(struct sys_info, f_6dd) == 0x6DD, "f_6dd");
_Static_assert(offsetof(struct sys_info, f_6de) == 0x6DE, "f_6de");
_Static_assert(offsetof(struct sys_info, f_6df) == 0x6DF, "f_6df");
_Static_assert(offsetof(struct sys_info, f_6e0) == 0x6E0, "f_6e0");
_Static_assert(offsetof(struct sys_info, f_6e1) == 0x6E1, "f_6e1");
_Static_assert(offsetof(struct sys_info, f_6e2) == 0x6E2, "f_6e2");
_Static_assert(offsetof(struct sys_info, f_6e3) == 0x6E3, "f_6e3");
_Static_assert(offsetof(struct sys_info, f_6e4) == 0x6E4, "f_6e4");
_Static_assert(offsetof(struct sys_info, f_6e5) == 0x6E5, "f_6e5");
_Static_assert(offsetof(struct sys_info, f_6e6) == 0x6E6, "f_6e6");
_Static_assert(offsetof(struct sys_info, f_6e7) == 0x6E7, "f_6e7");
_Static_assert(offsetof(struct sys_info, f_6e8) == 0x6E8, "f_6e8");
_Static_assert(offsetof(struct sys_info, f_6ec) == 0x6EC, "f_6ec");
_Static_assert(offsetof(struct sys_info, f_6ed) == 0x6ED, "f_6ed");
_Static_assert(offsetof(struct sys_info, runtime_ready) == 0x6EE, "runtime_ready");
_Static_assert(offsetof(struct sys_info, runtime_fpga_position) == 0x6F0, "runtime_fpga_position");
_Static_assert(offsetof(struct sys_info, runtime_position_base) == 0x6F2, "runtime_position_base");
_Static_assert(offsetof(struct sys_info, runtime_field_6f4) == 0x6F4, "runtime_field_6f4");
_Static_assert(offsetof(struct sys_info, runtime_step) == 0x6F6, "runtime_step");
_Static_assert(offsetof(struct sys_info, runtime_coarse_step) == 0x6F8, "runtime_coarse_step");
_Static_assert(offsetof(struct sys_info, runtime_fine_progress) == 0x6FC, "runtime_fine_progress");
_Static_assert(offsetof(struct sys_info, runtime_coarse_progress) == 0x700, "runtime_coarse_progress");
_Static_assert(offsetof(struct sys_info, runtime_field_704) == 0x704, "runtime_field_704");
_Static_assert(offsetof(struct sys_info, runtime_progress_threshold) == 0x708, "runtime_progress_threshold");
_Static_assert(offsetof(struct sys_info, step_limiter_on) == 0x70D, "step_limiter_on");
_Static_assert(offsetof(struct sys_info, f_70e) == 0x70E, "f_70e");
_Static_assert(offsetof(struct sys_info, f_710) == 0x710, "f_710");
_Static_assert(offsetof(struct sys_info, f_754) == 0x754, "f_754");
_Static_assert(offsetof(struct sys_info, f_756) == 0x756, "f_756");
_Static_assert(offsetof(struct sys_info, capture_slot_count) == 0x758, "capture_slot_count");
_Static_assert(offsetof(struct sys_info, rec_h_start) == 0x75C, "rec_h_start");
_Static_assert(offsetof(struct sys_info, rec_i_start) == 0x760, "rec_i_start");
_Static_assert(offsetof(struct sys_info, rec_j_start) == 0x764, "rec_j_start");
_Static_assert(offsetof(struct sys_info, rec_h_size) == 0x768, "rec_h_size");
_Static_assert(offsetof(struct sys_info, rec_i_size) == 0x76C, "rec_i_size");
_Static_assert(offsetof(struct sys_info, rec_j_size) == 0x770, "rec_j_size");
_Static_assert(offsetof(struct sys_info, rec_hij_end) == 0x774, "rec_hij_end");
_Static_assert(offsetof(struct sys_info, battery_level) == 0x778, "battery_level");
_Static_assert(offsetof(struct sys_info, charging) == 0x779, "charging");
_Static_assert(offsetof(struct sys_info, f_77a) == 0x77A, "f_77a");
_Static_assert(offsetof(struct sys_info, f_77c) == 0x77C, "f_77c");
_Static_assert(offsetof(struct sys_info, f_780) == 0x780, "f_780");
_Static_assert(offsetof(struct sys_info, f_784) == 0x784, "f_784");
_Static_assert(offsetof(struct sys_info, f_788) == 0x788, "f_788");
_Static_assert(offsetof(struct sys_info, menu_language) == 0x7AC, "menu_language");
_Static_assert(offsetof(struct sys_info, intensity_divisor) == 0x7B0, "intensity_divisor");
_Static_assert(offsetof(struct sys_info, ch1_buffer) == 0x7B4, "ch1_buffer");
_Static_assert(offsetof(struct sys_info, ch2_buffer) == 0x7B8, "ch2_buffer");
_Static_assert(offsetof(struct sys_info, capture_samples) == 0x7BC, "capture_samples");
_Static_assert(offsetof(struct sys_info, settings_slot) == 0x7C0, "settings_slot");
_Static_assert(offsetof(struct sys_info, rec_f_flash_addr) == 0x7C4, "rec_f_flash_addr");
_Static_assert(offsetof(struct sys_info, f_7c8) == 0x7C8, "f_7c8");
_Static_assert(offsetof(struct sys_info, f_7cc) == 0x7CC, "f_7cc");
_Static_assert(offsetof(struct sys_info, f_7d0) == 0x7D0, "f_7d0");
_Static_assert(offsetof(struct sys_info, f_7d4) == 0x7D4, "f_7d4");
_Static_assert(offsetof(struct sys_info, f_7d8) == 0x7D8, "f_7d8");
_Static_assert(offsetof(struct sys_info, rec_e_flash_addr) == 0x7DC, "rec_e_flash_addr");
_Static_assert(offsetof(struct sys_info, calibration_flash_addr) == 0x7E0, "calibration_flash_addr");
_Static_assert(offsetof(struct sys_info, rec_b_flash_addr) == 0x7E4, "rec_b_flash_addr");
_Static_assert(offsetof(struct sys_info, rec_g_flash_addr) == 0x7E8, "rec_g_flash_addr");
_Static_assert(offsetof(struct sys_info, rec_d_flash_addr) == 0x7EC, "rec_d_flash_addr");
_Static_assert(offsetof(struct sys_info, rec_c_flash_addr) == 0x7F0, "rec_c_flash_addr");
_Static_assert(offsetof(struct sys_info, rec_a_flash_addr) == 0x7F4, "rec_a_flash_addr");
_Static_assert(offsetof(struct sys_info, fft_size) == 0x7F8, "fft_size");
_Static_assert(offsetof(struct sys_info, fpga_probe_result) == 0x7FC, "fpga_probe_result");
_Static_assert(offsetof(struct sys_info, f_800) == 0x800, "f_800");
_Static_assert(offsetof(struct sys_info, f_801) == 0x801, "f_801");
_Static_assert(offsetof(struct sys_info, cursor_x_on) == 0x802, "cursor_x_on");
_Static_assert(offsetof(struct sys_info, cursor_y_on) == 0x803, "cursor_y_on");
_Static_assert(offsetof(struct sys_info, f_804) == 0x804, "f_804");
_Static_assert(offsetof(struct sys_info, cursor_x1) == 0x808, "cursor_x1");
_Static_assert(offsetof(struct sys_info, cursor_x2) == 0x80C, "cursor_x2");
_Static_assert(offsetof(struct sys_info, cursor_y1) == 0x810, "cursor_y1");
_Static_assert(offsetof(struct sys_info, cursor_y2) == 0x814, "cursor_y2");
_Static_assert(offsetof(struct sys_info, f_828) == 0x828, "f_828");
_Static_assert(offsetof(struct sys_info, f_830) == 0x830, "f_830");
_Static_assert(offsetof(struct sys_info, f_868) == 0x868, "f_868");
_Static_assert(offsetof(struct sys_info, ch1_avg_count) == 0x86A, "ch1_avg_count");
_Static_assert(offsetof(struct sys_info, ch2_avg_count) == 0x86C, "ch2_avg_count");
_Static_assert(offsetof(struct sys_info, ch1_avg_limit) == 0x86E, "ch1_avg_limit");
_Static_assert(offsetof(struct sys_info, ch2_avg_limit) == 0x870, "ch2_avg_limit");
_Static_assert(offsetof(struct sys_info, ch1_avg_ready) == 0x872, "ch1_avg_ready");
_Static_assert(offsetof(struct sys_info, ch2_avg_ready) == 0x873, "ch2_avg_ready");
_Static_assert(offsetof(struct sys_info, f_874) == 0x874, "f_874");
_Static_assert(offsetof(struct sys_info, f_878) == 0x878, "f_878");
_Static_assert(offsetof(struct sys_info, f_87c) == 0x87C, "f_87c");
_Static_assert(offsetof(struct sys_info, f_8e4) == 0x8E4, "f_8e4");
_Static_assert(offsetof(struct sys_info, f_8f8) == 0x8F8, "f_8f8");
_Static_assert(offsetof(struct sys_info, f_910) == 0x910, "f_910");
_Static_assert(offsetof(struct sys_info, f_958) == 0x958, "f_958");
_Static_assert(offsetof(struct sys_info, f_970) == 0x970, "f_970");
_Static_assert(offsetof(struct sys_info, ch1_amplitude) == 0xB30, "ch1_amplitude");
_Static_assert(offsetof(struct sys_info, ch1_mean_pos) == 0xB34, "ch1_mean_pos");
_Static_assert(offsetof(struct sys_info, ch2_amplitude) == 0xB4C, "ch2_amplitude");
_Static_assert(offsetof(struct sys_info, ch2_mean_pos) == 0xB50, "ch2_mean_pos");
_Static_assert(offsetof(struct sys_info, ch1_top_ref) == 0xB60, "ch1_top_ref");
_Static_assert(offsetof(struct sys_info, ch1_bottom_ref) == 0xB64, "ch1_bottom_ref");
_Static_assert(offsetof(struct sys_info, f_b68) == 0xB68, "f_b68");
_Static_assert(offsetof(struct sys_info, ch1_mid_level) == 0xB6C, "ch1_mid_level");
_Static_assert(offsetof(struct sys_info, f_b78) == 0xB78, "f_b78");
_Static_assert(offsetof(struct sys_info, f_b7c) == 0xB7C, "f_b7c");
_Static_assert(offsetof(struct sys_info, f_b88) == 0xB88, "f_b88");
_Static_assert(offsetof(struct sys_info, f_b8c) == 0xB8C, "f_b8c");
_Static_assert(offsetof(struct sys_info, f_b90) == 0xB90, "f_b90");
_Static_assert(offsetof(struct sys_info, f_b94) == 0xB94, "f_b94");
_Static_assert(offsetof(struct sys_info, f_ba0) == 0xBA0, "f_ba0");
_Static_assert(offsetof(struct sys_info, f_ba4) == 0xBA4, "f_ba4");
_Static_assert(offsetof(struct sys_info, outputs_applied) == 0xBB0, "outputs_applied");
_Static_assert(offsetof(struct sys_info, f_bb1) == 0xBB1, "f_bb1");
_Static_assert(offsetof(struct sys_info, f_bb2) == 0xBB2, "f_bb2");
_Static_assert(offsetof(struct sys_info, f_bb3) == 0xBB3, "f_bb3");
_Static_assert(offsetof(struct sys_info, f_bb4) == 0xBB4, "f_bb4");
_Static_assert(offsetof(struct sys_info, f_bb8) == 0xBB8, "f_bb8");
_Static_assert(offsetof(struct sys_info, f_bbc) == 0xBBC, "f_bbc");
_Static_assert(offsetof(struct sys_info, f_bc0) == 0xBC0, "f_bc0");
_Static_assert(offsetof(struct sys_info, f_bc4) == 0xBC4, "f_bc4");
_Static_assert(offsetof(struct sys_info, f_bc8) == 0xBC8, "f_bc8");
_Static_assert(offsetof(struct sys_info, f_bc9) == 0xBC9, "f_bc9");
_Static_assert(offsetof(struct sys_info, f_bca) == 0xBCA, "f_bca");
_Static_assert(offsetof(struct sys_info, timebase_steps) == 0xBD0, "timebase_steps");
_Static_assert(offsetof(struct sys_info, f_bd8) == 0xBD8, "f_bd8");
_Static_assert(offsetof(struct sys_info, f_be0) == 0xBE0, "f_be0");
_Static_assert(offsetof(struct sys_info, f_be8) == 0xBE8, "f_be8");
_Static_assert(offsetof(struct sys_info, f_bf0) == 0xBF0, "f_bf0");
_Static_assert(offsetof(struct sys_info, f_bf8) == 0xBF8, "f_bf8");
_Static_assert(offsetof(struct sys_info, f_c00) == 0xC00, "f_c00");
_Static_assert(offsetof(struct sys_info, f_c08) == 0xC08, "f_c08");
_Static_assert(offsetof(struct sys_info, f_c10) == 0xC10, "f_c10");
_Static_assert(offsetof(struct sys_info, f_c18) == 0xC18, "f_c18");
_Static_assert(offsetof(struct sys_info, f_c20) == 0xC20, "f_c20");
_Static_assert(offsetof(struct sys_info, f_c28) == 0xC28, "f_c28");
_Static_assert(offsetof(struct sys_info, f_c30) == 0xC30, "f_c30");
_Static_assert(offsetof(struct sys_info, f_c38) == 0xC38, "f_c38");
_Static_assert(offsetof(struct sys_info, f_c40) == 0xC40, "f_c40");
_Static_assert(offsetof(struct sys_info, f_c48) == 0xC48, "f_c48");
_Static_assert(offsetof(struct sys_info, f_c50) == 0xC50, "f_c50");
_Static_assert(offsetof(struct sys_info, f_c58) == 0xC58, "f_c58");
_Static_assert(offsetof(struct sys_info, f_c60) == 0xC60, "f_c60");
_Static_assert(offsetof(struct sys_info, f_c68) == 0xC68, "f_c68");
_Static_assert(offsetof(struct sys_info, f_c70) == 0xC70, "f_c70");
_Static_assert(offsetof(struct sys_info, f_c78) == 0xC78, "f_c78");
_Static_assert(offsetof(struct sys_info, f_c80) == 0xC80, "f_c80");
_Static_assert(offsetof(struct sys_info, f_c88) == 0xC88, "f_c88");
_Static_assert(offsetof(struct sys_info, f_c90) == 0xC90, "f_c90");
_Static_assert(offsetof(struct sys_info, f_c98) == 0xC98, "f_c98");
_Static_assert(offsetof(struct sys_info, f_ca0) == 0xCA0, "f_ca0");
_Static_assert(offsetof(struct sys_info, f_ca8) == 0xCA8, "f_ca8");
_Static_assert(offsetof(struct sys_info, f_cb0) == 0xCB0, "f_cb0");
_Static_assert(offsetof(struct sys_info, f_cb8) == 0xCB8, "f_cb8");
_Static_assert(offsetof(struct sys_info, f_cc0) == 0xCC0, "f_cc0");
_Static_assert(offsetof(struct sys_info, f_cc8) == 0xCC8, "f_cc8");
_Static_assert(offsetof(struct sys_info, f_cd0) == 0xCD0, "f_cd0");
_Static_assert(offsetof(struct sys_info, f_cd8) == 0xCD8, "f_cd8");
_Static_assert(offsetof(struct sys_info, f_ce0) == 0xCE0, "f_ce0");
_Static_assert(offsetof(struct sys_info, f_ce8) == 0xCE8, "f_ce8");
_Static_assert(offsetof(struct sys_info, f_cf0) == 0xCF0, "f_cf0");
_Static_assert(offsetof(struct sys_info, f_cf8) == 0xCF8, "f_cf8");
_Static_assert(offsetof(struct sys_info, f_d00) == 0xD00, "f_d00");
_Static_assert(offsetof(struct sys_info, f_d08) == 0xD08, "f_d08");
_Static_assert(offsetof(struct sys_info, f_d10) == 0xD10, "f_d10");
_Static_assert(offsetof(struct sys_info, f_d18) == 0xD18, "f_d18");
_Static_assert(offsetof(struct sys_info, f_d20) == 0xD20, "f_d20");
_Static_assert(offsetof(struct sys_info, list_mark_mode) == 0xF90, "list_mark_mode");
_Static_assert(offsetof(struct sys_info, list_page_size) == 0xF9A, "list_page_size");
_Static_assert(offsetof(struct sys_info, f_f9c) == 0xF9C, "f_f9c");
_Static_assert(offsetof(struct sys_info, list_page_index) == 0xF9E, "list_page_index");
_Static_assert(offsetof(struct sys_info, list_count) == 0xFA0, "list_count");
_Static_assert(offsetof(struct sys_info, list_page_count) == 0xFA2, "list_page_count");
_Static_assert(offsetof(struct sys_info, list_selected) == 0xFA4, "list_selected");
_Static_assert(offsetof(struct sys_info, f_fa6) == 0xFA6, "f_fa6");
_Static_assert(offsetof(struct sys_info, list_marked_count) == 0x88BE, "list_marked_count");
_Static_assert(offsetof(struct sys_info, path_buf) == 0x88C0, "path_buf");
_Static_assert(offsetof(struct sys_info, path_name) == 0x88C1, "path_name");
_Static_assert(offsetof(struct sys_info, f_88c9) == 0x88C9, "f_88c9");
_Static_assert(offsetof(struct sys_info, f_88cc) == 0x88CC, "f_88cc");
_Static_assert(offsetof(struct sys_info, bmp_width) == 0x109684, "bmp_width");
_Static_assert(offsetof(struct sys_info, bmp_height) == 0x109686, "bmp_height");
_Static_assert(offsetof(struct sys_info, time_cursors_on) == 0x3DA52C, "time_cursors_on");
_Static_assert(offsetof(struct sys_info, list_current) == 0x3DA52E, "list_current");
_Static_assert(offsetof(struct sys_info, f_3da530) == 0x3DA530, "f_3da530");
_Static_assert(offsetof(struct sys_info, cursor_a) == 0x3DA534, "cursor_a");
_Static_assert(offsetof(struct sys_info, cursor_b) == 0x3DA538, "cursor_b");
_Static_assert(offsetof(struct sys_info, capture_len) == 0x3DA53C, "capture_len");
_Static_assert(offsetof(struct sys_info, cursor_gap) == 0x3DA540, "cursor_gap");
_Static_assert(offsetof(struct sys_info, capture_cells) == 0x3DA544, "capture_cells");
_Static_assert(offsetof(struct sys_info, theme_pressed) == 0x3DD54C, "theme_pressed");
_Static_assert(offsetof(struct sys_info, theme_background) == 0x3DD550, "theme_background");
_Static_assert(offsetof(struct sys_info, theme_text) == 0x3DD554, "theme_text");

#define SI(f) (*(volatile __typeof__(((struct sys_info *)0)->f) *)(SYS_INFO_BASE + offsetof(struct sys_info, f)))

#endif
