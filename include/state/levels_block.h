/* struct levels_block: the level (spectrum) view block (backed up, 0x198 bytes), at 0x42E68668 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level and source; the others keep their offset as their
 * name. Access a field with LEVELS(field), a volatile access at the block address + offsetof, like
 * SI(field) for SYS_INFO (see include/sys_info.h). */
#ifndef LEVELS_BLOCK_H
#define LEVELS_BLOCK_H

#include <stddef.h>

#define LEVELS_BASE 0x42E68668u

struct levels_block {
    unsigned char _pad0[0x1];
    unsigned char panel_a_on;            /* +0x001 DERIVED: left side panel shown; gate of level_stats_60008 */
    unsigned char panel_b_on;            /* +0x002 DERIVED: second side panel shown; gate of level_stats_61000 */
    unsigned char move_slow;             /* +0x003 DERIVED: 'Movement': Fast move / Slow move */
    unsigned char f_004;                 /* +0x004 */
    unsigned char f_005;                 /* +0x005 */
    unsigned char _pad1[0x2];
    unsigned int max_freq;               /* +0x008 DERIVED: frequency of the "Max = " line */
    unsigned char _pad2[0x4];
    double max_level_dbmv;               /* +0x010 DERIVED: level of the "Max = " line, dBmV */
    double max_volts;                    /* +0x018 DERIVED: voltage of the "Max = " line */
    double f_020;                        /* +0x020 */
    double f_028;                        /* +0x028 */
    unsigned char _pad3[0x18];
    double f_048;                        /* +0x048 */
    unsigned char _pad4[0x8];
    unsigned int markers_on;             /* +0x058 DERIVED: frequency markers shown */
    int marker1_x;                       /* +0x05C DERIVED: marker C1 x offset in the plot */
    int marker2_x;                       /* +0x060 DERIVED: marker C2 x offset in the plot */
    unsigned char _pad5[0x4];
    long long c1_freq;                   /* +0x068 DERIVED: marker readout C1: frequency */
    long long c2_freq;                   /* +0x070 DERIVED: marker readout C2: frequency */
    long long dc_freq;                   /* +0x078 DERIVED: marker readout DC: frequency */
    double c1_level;                     /* +0x080 DERIVED: marker readout C1: level, dBmV */
    double c2_level;                     /* +0x088 DERIVED: marker readout C2: level, dBmV */
    double dc_level;                     /* +0x090 DERIVED: marker readout DC: level, dBmV */
    double c1_volts;                     /* +0x098 DERIVED: marker readout C1: voltage */
    double c2_volts;                     /* +0x0A0 DERIVED: marker readout C2: voltage */
    double dc_volts;                     /* +0x0A8 DERIVED: marker readout DC: voltage */
    unsigned int plot_x;                 /* +0x0B0 DERIVED: level plot box */
    unsigned int plot_y;                 /* +0x0B4 DERIVED: level plot box */
    unsigned int plot_w;                 /* +0x0B8 DERIVED: level plot box (s32 width) */
    unsigned int f_0bc;                  /* +0x0BC */
    unsigned int plot_h;                 /* +0x0C0 DERIVED: level plot box */
    unsigned int grid_x_step;            /* +0x0C4 DERIVED: pixels between vertical grid lines */
    unsigned int grid_x_div;             /* +0x0C8 DERIVED: vertical grid divisions */
    unsigned int grid_y_step;            /* +0x0CC DERIVED: pixels between horizontal grid lines */
    unsigned int grid_y_div;             /* +0x0D0 DERIVED: horizontal grid divisions */
    unsigned int panel_a_x;              /* +0x0D4 DERIVED: left side panel position */
    unsigned int panel_a_y;              /* +0x0D8 DERIVED: left side panel position */
    unsigned int f_0dc;                  /* +0x0DC */
    unsigned int panel_a_rows;           /* +0x0E0 DERIVED: left side panel colour bar rows */
    unsigned int panel_b_x;              /* +0x0E4 DERIVED: second side panel position */
    unsigned int panel_b_y;              /* +0x0E8 DERIVED: second side panel position */
    unsigned int f_0ec;                  /* +0x0EC */
    unsigned int panel_b_rows;           /* +0x0F0 DERIVED: second side panel colour bar rows */
    unsigned int spur_mode;              /* +0x0F4 DERIVED: FFT spur list mode 0..3 (also the calibration channel index) */
    unsigned int fft_points;             /* +0x0F8 DERIVED: number of FFT input floats */
    unsigned char _pad6[0x4];
    long long bin_width;                 /* +0x100 DERIVED: s64 frequency per bin */
    long long f_108;                     /* +0x108 */
    long long f_110;                     /* +0x110 */
    long long f_118;                     /* +0x118 */
    long long start_freq;                /* +0x120 DERIVED: 'Start frequency' */
    long long stop_freq;                 /* +0x128 DERIVED: 'Stop frequency' */
    long long center_freq;               /* +0x130 DERIVED: 'Center freq' */
    long long freq_span;                 /* +0x138 DERIVED: 'Freq width' */
    double attenuation;                  /* +0x140 DERIVED: 'Attenuation', dBmV */
    double reference;                    /* +0x148 DERIVED: 'Reference', dBmV */
    unsigned char _pad7[0x10];
    int f_160;                           /* +0x160 */
    int f_164;                           /* +0x164 */
    int f_168;                           /* +0x168 */
    int f_16c;                           /* +0x16C */
    int f_170;                           /* +0x170 */
    int f_174;                           /* +0x174 */
    int f_178;                           /* +0x178 */
    int f_17c;                           /* +0x17C */
    int f_180;                           /* +0x180 */
    unsigned int f_184;                  /* +0x184 */
    int f_188;                           /* +0x188 */
    unsigned int window_mode;            /* +0x18C DERIVED: how window_64_recalc keeps start / stop / centre / span consistent */
    unsigned char f_190;                 /* +0x190 */
};

_Static_assert(offsetof(struct levels_block, panel_a_on) == 0x1, "panel_a_on");
_Static_assert(offsetof(struct levels_block, panel_b_on) == 0x2, "panel_b_on");
_Static_assert(offsetof(struct levels_block, move_slow) == 0x3, "move_slow");
_Static_assert(offsetof(struct levels_block, f_004) == 0x4, "f_004");
_Static_assert(offsetof(struct levels_block, f_005) == 0x5, "f_005");
_Static_assert(offsetof(struct levels_block, max_freq) == 0x8, "max_freq");
_Static_assert(offsetof(struct levels_block, max_level_dbmv) == 0x10, "max_level_dbmv");
_Static_assert(offsetof(struct levels_block, max_volts) == 0x18, "max_volts");
_Static_assert(offsetof(struct levels_block, f_020) == 0x20, "f_020");
_Static_assert(offsetof(struct levels_block, f_028) == 0x28, "f_028");
_Static_assert(offsetof(struct levels_block, f_048) == 0x48, "f_048");
_Static_assert(offsetof(struct levels_block, markers_on) == 0x58, "markers_on");
_Static_assert(offsetof(struct levels_block, marker1_x) == 0x5C, "marker1_x");
_Static_assert(offsetof(struct levels_block, marker2_x) == 0x60, "marker2_x");
_Static_assert(offsetof(struct levels_block, c1_freq) == 0x68, "c1_freq");
_Static_assert(offsetof(struct levels_block, c2_freq) == 0x70, "c2_freq");
_Static_assert(offsetof(struct levels_block, dc_freq) == 0x78, "dc_freq");
_Static_assert(offsetof(struct levels_block, c1_level) == 0x80, "c1_level");
_Static_assert(offsetof(struct levels_block, c2_level) == 0x88, "c2_level");
_Static_assert(offsetof(struct levels_block, dc_level) == 0x90, "dc_level");
_Static_assert(offsetof(struct levels_block, c1_volts) == 0x98, "c1_volts");
_Static_assert(offsetof(struct levels_block, c2_volts) == 0xA0, "c2_volts");
_Static_assert(offsetof(struct levels_block, dc_volts) == 0xA8, "dc_volts");
_Static_assert(offsetof(struct levels_block, plot_x) == 0xB0, "plot_x");
_Static_assert(offsetof(struct levels_block, plot_y) == 0xB4, "plot_y");
_Static_assert(offsetof(struct levels_block, plot_w) == 0xB8, "plot_w");
_Static_assert(offsetof(struct levels_block, f_0bc) == 0xBC, "f_0bc");
_Static_assert(offsetof(struct levels_block, plot_h) == 0xC0, "plot_h");
_Static_assert(offsetof(struct levels_block, grid_x_step) == 0xC4, "grid_x_step");
_Static_assert(offsetof(struct levels_block, grid_x_div) == 0xC8, "grid_x_div");
_Static_assert(offsetof(struct levels_block, grid_y_step) == 0xCC, "grid_y_step");
_Static_assert(offsetof(struct levels_block, grid_y_div) == 0xD0, "grid_y_div");
_Static_assert(offsetof(struct levels_block, panel_a_x) == 0xD4, "panel_a_x");
_Static_assert(offsetof(struct levels_block, panel_a_y) == 0xD8, "panel_a_y");
_Static_assert(offsetof(struct levels_block, f_0dc) == 0xDC, "f_0dc");
_Static_assert(offsetof(struct levels_block, panel_a_rows) == 0xE0, "panel_a_rows");
_Static_assert(offsetof(struct levels_block, panel_b_x) == 0xE4, "panel_b_x");
_Static_assert(offsetof(struct levels_block, panel_b_y) == 0xE8, "panel_b_y");
_Static_assert(offsetof(struct levels_block, f_0ec) == 0xEC, "f_0ec");
_Static_assert(offsetof(struct levels_block, panel_b_rows) == 0xF0, "panel_b_rows");
_Static_assert(offsetof(struct levels_block, spur_mode) == 0xF4, "spur_mode");
_Static_assert(offsetof(struct levels_block, fft_points) == 0xF8, "fft_points");
_Static_assert(offsetof(struct levels_block, bin_width) == 0x100, "bin_width");
_Static_assert(offsetof(struct levels_block, f_108) == 0x108, "f_108");
_Static_assert(offsetof(struct levels_block, f_110) == 0x110, "f_110");
_Static_assert(offsetof(struct levels_block, f_118) == 0x118, "f_118");
_Static_assert(offsetof(struct levels_block, start_freq) == 0x120, "start_freq");
_Static_assert(offsetof(struct levels_block, stop_freq) == 0x128, "stop_freq");
_Static_assert(offsetof(struct levels_block, center_freq) == 0x130, "center_freq");
_Static_assert(offsetof(struct levels_block, freq_span) == 0x138, "freq_span");
_Static_assert(offsetof(struct levels_block, attenuation) == 0x140, "attenuation");
_Static_assert(offsetof(struct levels_block, reference) == 0x148, "reference");
_Static_assert(offsetof(struct levels_block, f_160) == 0x160, "f_160");
_Static_assert(offsetof(struct levels_block, f_164) == 0x164, "f_164");
_Static_assert(offsetof(struct levels_block, f_168) == 0x168, "f_168");
_Static_assert(offsetof(struct levels_block, f_16c) == 0x16C, "f_16c");
_Static_assert(offsetof(struct levels_block, f_170) == 0x170, "f_170");
_Static_assert(offsetof(struct levels_block, f_174) == 0x174, "f_174");
_Static_assert(offsetof(struct levels_block, f_178) == 0x178, "f_178");
_Static_assert(offsetof(struct levels_block, f_17c) == 0x17C, "f_17c");
_Static_assert(offsetof(struct levels_block, f_180) == 0x180, "f_180");
_Static_assert(offsetof(struct levels_block, f_184) == 0x184, "f_184");
_Static_assert(offsetof(struct levels_block, f_188) == 0x188, "f_188");
_Static_assert(offsetof(struct levels_block, window_mode) == 0x18C, "window_mode");
_Static_assert(offsetof(struct levels_block, f_190) == 0x190, "f_190");

#define LEVELS(f) (*(volatile __typeof__(((struct levels_block *)0)->f) *)(LEVELS_BASE + offsetof(struct levels_block, f)))

#endif
