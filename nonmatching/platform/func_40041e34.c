/*
 * fft_traces_draw -- stock 0x40041E34 (288 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the FFT traces of the enabled channels: for each x below width - 1 a vertical segment between the s16 values k and k + 1 (0x42E5B014.., channel 2 at 0x42E5C014..) plus the trace origin (y SYS+6; channel 2 adds half the height SYS+0xA), in 0xFF8800 and 0x88FF.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);

static void trace(volatile short *p, u32 x0, u32 y0)
{
    u32 i;
    for (i = 0; i < SYS16(8) - 1u; i++, p++) {
        int a = p[0], b = p[1];
        u32 x = (unsigned short)(i + x0);
        if (b < a)
            vline_draw(x, (unsigned short)(b + y0), (unsigned short)(a + y0));
        else
            vline_draw(x, (unsigned short)(a + y0), (unsigned short)(b + y0));
    }
}

void fft_traces_draw(void)
{
    if (SYS8(0x3E) && SYS8(0x42)) {
        u32 x0 = SYS16(4), y0 = SYS16(6);
        draw_colour_set(0xFF8800);
        trace((volatile short *)0x42E5B014u, x0, y0);
    }
    if (SYS8(0x102) && SYS8(0x106)) {
        u32 y0 = SYS16(6) + (SYS16(0xA) >> 1), x0 = SYS16(4);
        draw_colour_set(0x88FF);
        trace((volatile short *)0x42E5C014u, x0, y0);
    }
}
