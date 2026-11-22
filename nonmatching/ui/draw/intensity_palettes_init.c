/*
 * intensity_palettes_init -- stock 0x4005DB9C (568 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Builds the RGB565 palettes of the intensity view. Palette A (0x42E45864, three 1500-entry parts): yellow 0xFFE0 / cyan 0x7FF / green 0x7E0 fills, each starting with a 64-step ramp from black. Palette B (0x42E47B8C): a 192-step blue - cyan - green - yellow - red ramp over a red 0xF800 fill of 1000 entries, copied to its parts at +3000 and +6000 bytes. Then two u32 colour tables, both filled with 0xFFFF first: 0x42E56C30 = the ramps 0xFF .. 0xFFFF .. 0xFF00 .. 0xFFFF00 .. 0xFF0000 (1021 words, past its 1000-word fill) and 0x42E583A0 = 0x33 .. 0xFF .. 0xFFFF (460 words).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "util.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))
#define D64(a) (*(volatile double *)(a))

#define PAL_A 0x42E45864u
#define PAL_B 0x42E47B8Cu
#define TAB_A 0x42E56C30u
#define TAB_B 0x42E583A0u

void intensity_palettes_init(void)
{
    volatile unsigned short *pa = (volatile unsigned short *)PAL_A, *pb = (volatile unsigned short *)PAL_B;
    volatile u32 *t;
    int i;
    u32 v;
    fill_u16((unsigned short *)PAL_A, 0xFFE0, 1500);
    for (i = 0; i < 64; i++)
        pa[i] = (unsigned short)(((i / 2) << 11) + (i << 5));
    fill_u16((unsigned short *)(PAL_A + 3000), 0x7FF, 1500);
    for (i = 0; i < 64; i++)
        pa[1500 + i] = (unsigned short)((i / 2) + (i << 5));
    fill_u16((unsigned short *)(PAL_A + 6000), 0x7E0, 1500);
    for (i = 0; i < 64; i++)
        pa[3000 + i] = (unsigned short)(i << 5);
    fill_u16((unsigned short *)PAL_B, 0xF800, 1000);
    i = 0;
    for (v = 0x1F; v <= 0x7FF; v += 0x20)
        pb[i++] = (unsigned short)v;
    for (v = 0x7FF; v >= 0x7E0; v--)
        pb[i++] = (unsigned short)v;
    for (v = 0x7E0; v <= 0xFFE0; v += 0x800)
        pb[i++] = (unsigned short)v;
    for (v = 0xFFE0; v >= 0xF800; v -= 0x20)
        pb[i++] = (unsigned short)v;
    copy_u16((unsigned short *)(PAL_B + 3000), (const unsigned short *)PAL_B, 1000);
    copy_u16((unsigned short *)(PAL_B + 6000), (const unsigned short *)PAL_B, 1000);
    t = (volatile u32 *)TAB_A;
    fill_u32((u32 *)TAB_A, 0xFFFF, 1000);
    i = 0;
    t[0] = 0xFF;
    for (v = 0x1FF; v <= 0xFFFF; v += 0x100)
        t[++i] = v;
    for (v = 0xFFFE; v >= 0xFF00; v--)
        t[++i] = v;
    for (v = 0x1FF00; v <= 0xFFFF00; v += 0x10000)
        t[++i] = v;
    for (v = 0xFFFE00; v >= 0xFF0000; v -= 0x100)
        t[++i] = v;
    t = (volatile u32 *)TAB_B;
    fill_u32((u32 *)TAB_B, 0xFFFF, 1000);
    i = 0;
    t[0] = 0x33;
    for (v = 0x34; v <= 0xFF; v++)
        t[++i] = v;
    for (v = 0x1FF; v <= 0xFFFF; v += 0x100)
        t[++i] = v;
}
