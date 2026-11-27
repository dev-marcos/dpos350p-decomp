/*
 * fonts_init -- stock 0x4002CCD8 (236 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sets up the nine font descriptors (0x4C0 bytes each): byte +0 (1 or 8) and the glyph data pointer +0x4BC (in the graphics blob), then font_metrics_build on each.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void fonts_init(void)
{
    *(volatile unsigned char *)0x42E64F58u = 1;
    *(volatile u32 *)0x42E65414u = IMGU(4007da1c);
    *(volatile unsigned char *)0x42E65418u = 1;
    *(volatile u32 *)0x42E658D4u = IMGU(4007a4ec);
    *(volatile unsigned char *)0x42E64A98u = 1;
    *(volatile u32 *)0x42E64F54u = IMGU(40076c98);
    *(volatile unsigned char *)0x42E658D8u = 1;
    *(volatile u32 *)0x42E65D94u = IMGU(40072d78);
    *(volatile unsigned char *)0x42E65D98u = 1;
    *(volatile u32 *)0x42E66254u = IMGU(4006ddec);
    *(volatile unsigned char *)0x42E66258u = 8;
    *(volatile u32 *)0x42E66714u = IMGU(400841f0);
    *(volatile unsigned char *)0x42E66718u = 8;
    *(volatile u32 *)0x42E66BD4u = IMGU(400836e8);
    *(volatile unsigned char *)0x42E66BD8u = 8;
    *(volatile u32 *)0x42E67094u = IMGU(4008279c);
    *(volatile unsigned char *)0x42E67098u = 8;
    *(volatile u32 *)0x42E67554u = IMGU(400809d0);
    font_metrics_build((volatile unsigned char *)0x42E64F58u);
    font_metrics_build((volatile unsigned char *)0x42E65418u);
    font_metrics_build((volatile unsigned char *)0x42E64A98u);
    font_metrics_build((volatile unsigned char *)0x42E658D8u);
    font_metrics_build((volatile unsigned char *)0x42E65D98u);
    font_metrics_build((volatile unsigned char *)0x42E66258u);
    font_metrics_build((volatile unsigned char *)0x42E66718u);
    font_metrics_build((volatile unsigned char *)0x42E66BD8u);
    font_metrics_build((volatile unsigned char *)0x42E67098u);
}
