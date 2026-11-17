/*
 * level_columns_draw_b -- stock 0x40061194 (332 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Second level-column graph (enabled by byte 0x42E6866A, skipped while 0x42E68BC7 is set): for each of the M+0xEC columns, the rows from the height M+0xF0 up to the column top (u16 0x42E61000 + 2col) get the colour of table 0x42E583A0[(int)(i * 510.0 / (height - M+0x174))] (i counting up from the bottom) in the buffer 0x425948F4 (width M+0xEC); then the M+0xF0 rows of the buffer are copied to the screen *0x40095498 at row M+0xE8 + r (stride 1022 pixels), column M+0xE4. M = 0x42E68668.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define COLOUR16 (*(volatile unsigned short *)0x42E68660u)
#define SCREEN (*(volatile u32 *)0x40095498u)

#define TOP16(c) (*(volatile unsigned short *)(0x42E61000u + 2 * (c)))
#define TBL(i) (*(volatile u32 *)(0x42E583A0u + 4 * (i)))
#define BUF ((volatile unsigned short *)0x425948F4u)

void level_columns_draw_b(void)
{
    int col, r;
    u32 h;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (MB8(2) == 0)
        return;
    h = MB32(0xF0);
    for (col = 0; col < (int)MB32(0xEC); col++) {
        int top = TOP16(col), y = (int)h, i = 0;
        double k = 510.0 / (double)(u32)(h - MB32(0x174));
        if (y >= top) {
            do {
                draw_colour_set(TBL((int)((double)i * k)));
                i++;
                BUF[MB32(0xEC) * (u32)y + (u32)col] = COLOUR16;
                y--;
            } while (y >= top);
        }
    }
    for (r = 0; r < (int)MB32(0xF0); r++)
        copy_u16((unsigned short *)(SCREEN + ((u32)r + MB32(0xE8)) * 2044u + MB32(0xE4) * 2u),
                 (const unsigned short *)(BUF + MB32(0xEC) * (u32)r), MB32(0xEC));
}
