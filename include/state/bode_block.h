/* struct bode_block: the Bode (frequency response) view block (backed up, 0xAD20 bytes), at 0x42E180C8 (build 2025-06-19).
 *
 * Only the fields the C code reaches are listed; the gaps are padding. A named field carries its
 * evidence level and source (docs/DRIVERS.md, batches 50 and 51); the others keep their offset as
 * their name. Access a field with BODES(field), a volatile access at the block address + offsetof,
 * like SI(field) for SYS_INFO (see include/sys_info.h). The per-point arrays take an index:
 * BODES(gain_db[i]). The array lengths are DERIVED from the distance to the next field. */
#ifndef BODE_BLOCK_H
#define BODE_BLOCK_H

#include <stddef.h>

#define BODES_BASE 0x42E180C8u

struct bode_block {
    unsigned short plot_x;               /* +0x000 DERIVED: plot box x (70) */
    unsigned short plot_y;               /* +0x002 DERIVED: plot box y (48) */
    unsigned short plot_w;               /* +0x004 DERIVED: plot width in pixels (867) */
    unsigned short plot_h;               /* +0x006 DERIVED: plot height in pixels (480) */
    unsigned short decade_w;             /* +0x008 DERIVED: width of one frequency decade (289) */
    unsigned short decades;              /* +0x00A DERIVED: number of decades (3) */
    unsigned short row_h;                /* +0x00C DERIVED: height of one grid row */
    unsigned short rows;                 /* +0x00E DERIVED: number of grid rows */
    double gain_top;                     /* +0x010 DERIVED: gain axis top, dB */
    double gain_bottom;                  /* +0x018 DERIVED: gain axis bottom, dB */
    double phase_top;                    /* +0x020 DERIVED: phase axis top, degrees */
    double phase_bottom;                 /* +0x028 DERIVED: phase axis bottom, degrees */
    unsigned char _pad0[0x10];
    unsigned int zoom_factor;            /* +0x040 DERIVED: resampling factor kept by bode_curves_zoom */
    unsigned char loop;                  /* +0x044 DERIVED: 'Single' / 'Loop' */
    unsigned char running;               /* +0x045 DERIVED: run indicator of the Bode toolbar */
    unsigned char move_slow;             /* +0x046 DERIVED: 'Movement': Fast move / Slow move */
    unsigned char sweep_running;         /* +0x047 DERIVED: a sweep is running (touch byte +0x15 stops it) */
    int f_048;                           /* +0x048 OPEN: a mode (3 / 4 tie the two widths in trace_cursor_limits) */
    int amplitude;                       /* +0x04C DERIVED: 'Amplitude' (x 1000 uV) */
    int offset;                          /* +0x050 DERIVED: 'Offset' (x 1000 uV) */
    int start_freq;                      /* +0x054 DERIVED: 'Start frequency', Hz */
    int stop_freq;                       /* +0x058 DERIVED: 'Stop frequency', Hz */
    unsigned int freq_count;             /* +0x05C DERIVED: 'Frequency count' n (n + 1 points) */
    double freq_step;                    /* +0x060 DERIVED: linear step (stop - start) / n */
    double gain_ratio[2000];             /* +0x068 DERIVED: gain B / A of each point */
    double gain_db[2000];                /* +0x3EE8 DERIVED: 20 log10 of the ratio */
    unsigned short phase_deg[2000];      /* +0x7D68 DERIVED: phase in degrees (s16, read through (short)) */
    double log_start;                    /* +0x8D08 DERIVED: log10 of the start frequency */
    double log_end;                      /* +0x8D10 DERIVED: log10 of the stop frequency */
    double log_span;                     /* +0x8D18 DERIVED: log_end - log_start */
    unsigned short gain_rows[2000];      /* +0x8D20 DERIVED: pixel row of the gain curve per x */
    unsigned short phase_rows[2000];     /* +0x9CC0 DERIVED: pixel row of the phase curve per x */
    unsigned char cursors_on;            /* +0xAC60 DERIVED: Bode cursors drawn */
    unsigned char _pad1[0x3];
    int cursor1;                         /* +0xAC64 DERIVED: Bode cursor mark 1 */
    int cursor2;                         /* +0xAC68 DERIVED: Bode cursor mark 2 */
    unsigned char _pad2[0x4];
    double c1_freq;                      /* +0xAC70 DERIVED: cursor values (trace_cursor_values), */
    double c2_freq;                      /* +0xAC78 shown by bode_cursors_draw as the rows */
    double dc_freq;                      /* +0xAC80 "C1 = ", "C2 = " and "DC = " (the difference): */
    double c1_gain;                      /* +0xAC88 frequency, gain in dB, phase in degrees */
    double c2_gain;                      /* +0xAC90 */
    double dc_gain;                      /* +0xAC98 */
    double c1_phase;                     /* +0xACA0 */
    double c2_phase;                     /* +0xACA8 */
    double dc_phase;                     /* +0xACB0 */
    unsigned char _pad3[0x40];
    double amp_a;                        /* +0xACF8 DERIVED: amplitude of input A at the current point */
    double amp_b;                        /* +0xAD00 DERIVED: amplitude of input B, divided by cal_ratio */
    double cal_ratio;                    /* +0xAD08 DERIVED: calibration ratio of the current point */
    double cal_phase;                    /* +0xAD10 DERIVED: calibration phase offset of the current point */
    unsigned char sweep_busy;            /* +0xAD18 DERIVED: set during the points (no event service) */
    unsigned char a_reached;             /* +0xAD19 DERIVED: input A reached 5000 once in the sweep */
    unsigned char b_reached;             /* +0xAD1A DERIVED: input B reached 5000 once in the sweep */
};

_Static_assert(offsetof(struct bode_block, plot_x) == 0x0, "plot_x");
_Static_assert(offsetof(struct bode_block, plot_y) == 0x2, "plot_y");
_Static_assert(offsetof(struct bode_block, plot_w) == 0x4, "plot_w");
_Static_assert(offsetof(struct bode_block, plot_h) == 0x6, "plot_h");
_Static_assert(offsetof(struct bode_block, decade_w) == 0x8, "decade_w");
_Static_assert(offsetof(struct bode_block, decades) == 0xA, "decades");
_Static_assert(offsetof(struct bode_block, row_h) == 0xC, "row_h");
_Static_assert(offsetof(struct bode_block, rows) == 0xE, "rows");
_Static_assert(offsetof(struct bode_block, gain_top) == 0x10, "gain_top");
_Static_assert(offsetof(struct bode_block, gain_bottom) == 0x18, "gain_bottom");
_Static_assert(offsetof(struct bode_block, phase_top) == 0x20, "phase_top");
_Static_assert(offsetof(struct bode_block, phase_bottom) == 0x28, "phase_bottom");
_Static_assert(offsetof(struct bode_block, zoom_factor) == 0x40, "zoom_factor");
_Static_assert(offsetof(struct bode_block, loop) == 0x44, "loop");
_Static_assert(offsetof(struct bode_block, running) == 0x45, "running");
_Static_assert(offsetof(struct bode_block, move_slow) == 0x46, "move_slow");
_Static_assert(offsetof(struct bode_block, sweep_running) == 0x47, "sweep_running");
_Static_assert(offsetof(struct bode_block, f_048) == 0x48, "f_048");
_Static_assert(offsetof(struct bode_block, amplitude) == 0x4C, "amplitude");
_Static_assert(offsetof(struct bode_block, offset) == 0x50, "offset");
_Static_assert(offsetof(struct bode_block, start_freq) == 0x54, "start_freq");
_Static_assert(offsetof(struct bode_block, stop_freq) == 0x58, "stop_freq");
_Static_assert(offsetof(struct bode_block, freq_count) == 0x5C, "freq_count");
_Static_assert(offsetof(struct bode_block, freq_step) == 0x60, "freq_step");
_Static_assert(offsetof(struct bode_block, gain_ratio) == 0x68, "gain_ratio");
_Static_assert(offsetof(struct bode_block, gain_db) == 0x3EE8, "gain_db");
_Static_assert(offsetof(struct bode_block, phase_deg) == 0x7D68, "phase_deg");
_Static_assert(offsetof(struct bode_block, log_start) == 0x8D08, "log_start");
_Static_assert(offsetof(struct bode_block, log_end) == 0x8D10, "log_end");
_Static_assert(offsetof(struct bode_block, log_span) == 0x8D18, "log_span");
_Static_assert(offsetof(struct bode_block, gain_rows) == 0x8D20, "gain_rows");
_Static_assert(offsetof(struct bode_block, phase_rows) == 0x9CC0, "phase_rows");
_Static_assert(offsetof(struct bode_block, cursors_on) == 0xAC60, "cursors_on");
_Static_assert(offsetof(struct bode_block, cursor1) == 0xAC64, "cursor1");
_Static_assert(offsetof(struct bode_block, cursor2) == 0xAC68, "cursor2");
_Static_assert(offsetof(struct bode_block, c1_freq) == 0xAC70, "c1_freq");
_Static_assert(offsetof(struct bode_block, dc_phase) == 0xACB0, "dc_phase");
_Static_assert(offsetof(struct bode_block, amp_a) == 0xACF8, "amp_a");
_Static_assert(offsetof(struct bode_block, amp_b) == 0xAD00, "amp_b");
_Static_assert(offsetof(struct bode_block, cal_ratio) == 0xAD08, "cal_ratio");
_Static_assert(offsetof(struct bode_block, cal_phase) == 0xAD10, "cal_phase");
_Static_assert(offsetof(struct bode_block, sweep_busy) == 0xAD18, "sweep_busy");
_Static_assert(offsetof(struct bode_block, a_reached) == 0xAD19, "a_reached");
_Static_assert(offsetof(struct bode_block, b_reached) == 0xAD1A, "b_reached");
_Static_assert(sizeof(struct bode_block) <= 0xAD20, "size");

#define BODES(f) (*(volatile __typeof__(((struct bode_block *)0)->f) *)(BODES_BASE + offsetof(struct bode_block, f)))

#endif
