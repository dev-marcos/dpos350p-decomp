/*
 * auto_level_search -- stock 0x40060FE8 (420 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel 1 level search (DERIVED; skipped while byte 0x42E68BC7 is set): channel 1 on (SYS+0x3E, +0x3F = 1, SYS+0x102 = 0), offset SYS+0x60 = 250, SYS+0x46 = 0, SYS+0x54F = 1, SYS+0x275 = 1, SYS+0x70D = 0, SYS+0x660 = 0, range SYS+0x588 = 0xB, byte 0x42E68BC6 = 1. Then it applies the outputs and runs one acquisition per step, lowering the level byte SYS+0x41 (down to 0) while the measured word SYS+0xB30 is above 0.9 * height (SYS+0xA), raising it (up to 7) while it is below 0.2 * height. Afterwards the double 0x42E22DC0 = s64 SYS+0x910, fft_input_prepare_b runs and the two level-column buffers (0x42B8C194 width M+0xDC, rows M+0xE0; 0x425948F4 width M+0xEC, rows M+0xF0; when enabled by M+1 / M+2) are scrolled up by M+0x184 rows. M = 0x42E68668.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "libc.h"
#include "processing.h"
#include "system.h"
#include "ui/draw.h"
#include "util.h"

#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define COLOUR16 (*(volatile unsigned short *)0x42E68660u)
#define SCREEN (*(volatile u32 *)0x40095498u)

static void scroll_rows(u32 buf, u32 ow, u32 oh)
{
    int i;
    if ((int)(MB32(oh) - MB32(0x184)) < 0)
        return;
    i = 0;
    do {
        u32 w = MB32(ow);
        copy_u16((unsigned short *)(buf + 2 * (w * (u32)i)), (const unsigned short *)(buf + 2 * (w * (MB32(0x184) + (u32)i))), w);
        i++;
    } while ((int)(MB32(oh) - MB32(0x184)) >= i);
}

void auto_level_search(void)
{
    u32 h, hi, lo, v, l;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS8(0x3F) = 1;
    SYS32(0x60) = 0xFA;
    SYS8(0x46) = 0;
    SYS8(0x54F) = 1;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 1;
    SYS8(0x275) = 1;
    h = SYS16(0xA);
    SYS8(0x70D) = 0;
    SYS8(0x660) = 0;
    SYS8(0x588) = 0xB;
    hi = (u32)((double)h * 0.9);
    lo = (u32)((double)h * 0.2);
    for (;;) {
        outputs_apply();
        acquisition_dispatcher();
        v = SYS32(0xB30);
        if (hi < v) {
            l = SYS8(0x41);
            if (l == 0)
                break;
            SYS8(0x41) = l - 1;
        } else if (v < lo) {
            l = SYS8(0x41);
            if ((int)l >= 7)
                break;
            SYS8(0x41) = l + 1;
        } else {
            break;
        }
    }
    *(volatile double *)0x42E22DC0u = (double)*(volatile long long *)(0x40544E68u + 0x910);
    fft_input_prepare_b();
    if (MB8(1))
        scroll_rows(0x42B8C194u, 0xDC, 0xE0);
    if (MB8(2))
        scroll_rows(0x425948F4u, 0xEC, 0xF0);
}
