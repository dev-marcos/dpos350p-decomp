/*
 * fonts_init -- stock 0x4002CCD8 (236 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sets up the nine font descriptors (0x4C0 bytes each): byte +0 (1 or 8) and the glyph data pointer +0x4BC (in the graphics blob), then font_metrics_build on each.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void font_metrics_build(volatile unsigned char *d);

void fonts_init(void)
{
    *(volatile unsigned char *)0x42E64F58u = 1;
    *(volatile u32 *)0x42E65414u = 0x4007DA1Cu;
    *(volatile unsigned char *)0x42E65418u = 1;
    *(volatile u32 *)0x42E658D4u = 0x4007A4ECu;
    *(volatile unsigned char *)0x42E64A98u = 1;
    *(volatile u32 *)0x42E64F54u = 0x40076C98u;
    *(volatile unsigned char *)0x42E658D8u = 1;
    *(volatile u32 *)0x42E65D94u = 0x40072D78u;
    *(volatile unsigned char *)0x42E65D98u = 1;
    *(volatile u32 *)0x42E66254u = 0x4006DDECu;
    *(volatile unsigned char *)0x42E66258u = 8;
    *(volatile u32 *)0x42E66714u = 0x400841F0u;
    *(volatile unsigned char *)0x42E66718u = 8;
    *(volatile u32 *)0x42E66BD4u = 0x400836E8u;
    *(volatile unsigned char *)0x42E66BD8u = 8;
    *(volatile u32 *)0x42E67094u = 0x4008279Cu;
    *(volatile unsigned char *)0x42E67098u = 8;
    *(volatile u32 *)0x42E67554u = 0x400809D0u;
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
