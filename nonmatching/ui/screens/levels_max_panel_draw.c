/*
 * levels_max_panel_draw -- stock 0x40062A18 (240 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte 0x42E68BB0+0x17 is set, draws the 'Max = ' line of the level view at (W32 0x42E68668+0xB0 + 8, +0xB4 + 8) in colour 0xDDDDDD: the text, the frequency (s32 +8, drawn then measured), after a gap of 20 the level +0x10 in dBmV (level_label_dbmv flag 1) and then the voltage +0x18 x 1000 (voltage_uv_draw), each placed after the font advance u16 0x42E68BE4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/screens.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))
#define D64(a) (*(volatile double *)(a))

#define BLK 0x42E68668u
#define ADV 0x42E68BE4u

void levels_max_panel_draw(void)
{
    u32 x, y, w;
    if (B8(0x42E68BB0u + 0x17))
        return;
    x = W32(BLK + 0xB0) + 8;
    y = W32(BLK + 0xB4) + 8;
    half_42e68be2_set(1);
    word_42e68c74_set(0x42E64F58u);
    half_42e68be2_set(2);
    word_42e68c74_set(0x42E658D8u);
    draw_colour_set(0xDDDDDD);
    text_draw(x & 0xFFFF, y & 0xFFFF, (const char *)0x400902C0u);
    x += H16(ADV);
    frequency_draw(x & 0xFFFF, y & 0xFFFF, 0, (double)(int)W32(BLK + 8));
    w = frequency_draw(x & 0xFFFF, y & 0xFFFF, 1, (double)(int)W32(BLK + 8));
    x = w + x + 0x14;
    level_label_dbmv(x & 0xFFFF, y & 0xFFFF, 1, D64(BLK + 0x10));
    voltage_uv_draw((H16(ADV) + x + 0x3C) & 0xFFFF, y & 0xFFFF, 0, D64(BLK + 0x18) * 1000.0);
}
