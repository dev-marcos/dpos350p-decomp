/*
 * state_block_68668_init -- stock 0x4003EDF4 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Defaults of the block at 0x42E68668: +0xF4 / +0x188 = 2, +0x140 = 20.0, +0x148 = 60.0, the 64-bit values +0x120 = 200000, +0x128 = 500000000, +0x130 = 250100000, +0x138 = (+0x128) - 200000, +0x5C = (+0xB8) / 5 and +0x60 = 4 (+0xB8) / 5 (signed), several fields cleared; the byte 0x42E68BC6 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "state/levels_block.h"

#define H(o) (*(volatile unsigned short *)(0x42E68668u + (o)))
#define W(o) (*(volatile u32 *)(0x42E68668u + (o)))

void state_block_68668_init(void)
{
    int v;
    W(0x188) = 2;
    LEVELS(spur_mode) = 2;
    H(0) = 0;
    LEVELS(panel_b_on) = 0;
    LEVELS(f_190) = 0;
    LEVELS(attenuation) = 20.0;
    LEVELS(reference) = 60.0;
    W(0x120) = 200000;
    W(0x124) = 0;
    W(0x128) = 500000000;
    W(0x12C) = 0;
    W(0x130) = 250100000;
    W(0x134) = 0;
    W(0x13C) = 0;
    W(0x138) = W(0x128) - 200000;
    v = (int)LEVELS(plot_w);
    LEVELS(window_mode) = 0;
    LEVELS(f_004) = 0;
    LEVELS(markers_on) = 0;
    W(0x5C) = v / 5;
    W(0x60) = (int)((u32)v * 4u) / 5;
    g_touch->enabled = 1;
}
