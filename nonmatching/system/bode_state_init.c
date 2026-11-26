/*
 * bode_state_init -- stock 0x4003ED80 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Defaults of the block at 0x42E180C8 (+0x4C/+0x54 = 1000, +0x50 = 0, +0x58 = 50000000, +0x5C = 20, bytes +0xAC60 and +0xAD18 = 0, +0xAC64 = h / 5 and +0xAC68 = 4h / 5 with h = the halfword +4) and the byte 0x42E68BC6 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "state/bode_block.h"

#define B(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))

void bode_state_init(void)
{
    u32 h;
    BODES(freq_count) = 20;
    h = BODES(plot_w);
    BODES(stop_freq) = 50000000;
    BODES(start_freq) = 1000;
    BODES(amplitude) = 1000;
    BODES(cursors_on) = 0;
    BODES(offset) = 0;
    B(0xAC60 + 0xB8) = 0;
    BODES(cursor1) = h / 5u;
    BODES(cursor2) = (u32)((int)(h * 4) / 5);
    g_touch->enabled = 1;
}
