/*
 * level_columns_draw -- stock 0x40062C98 (352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Level-column graph (enabled by byte 0x42E68669, skipped while 0x42E68BC7 is set): for each of the M+0xDC columns and each step i from the height h = M+0xE0 down to the column top (s16 0x42E60008 + 2col), the colour of table 0x42E56C30[(int)(i * 1000.0 / h)] is set and M+0x184 pixels of the column are painted upwards from the bottom row of the buffer 0x42B8C194 (width M+0xDC; the start does not move with i, as in the stock code); then the M+0xE0 rows are copied to the screen *0x40095498 at row M+0xD8 + r (stride 1022 pixels), column M+0xD4. M = 0x42E68668.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "util.h"

#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define COLOUR16 (*(volatile unsigned short *)0x42E68660u)
#define SCREEN (*(volatile u32 *)0x40095498u)

#define TOP16(c) (*(volatile short *)(0x42E60008u + 2 * (c)))
#define TBL(i) (*(volatile u32 *)(0x42E56C30u + 4 * (i)))
#define BUF ((volatile unsigned short *)0x42B8C194u)

void level_columns_draw(void)
{
    u32 h0 = MB32(0xE0);
    double k;
    int col, r;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (MB8(1) == 0)
        return;
    k = 1000.0 / (double)h0;
    for (col = 0; col < (int)MB32(0xDC); col++) {
        int top = TOP16(col), y = (int)h0, i = 0;
        if (y >= top) {
            do {
                u32 w;
                int n;
                volatile unsigned short *p;
                draw_colour_set(TBL((int)((double)i * k)));
                w = MB32(0xDC);
                p = BUF + (w * MB32(0xE0) + (u32)col);
                n = (int)MB32(0x184);
                if (n > 0) {
                    unsigned short c = COLOUR16;
                    do {
                        *p = c;
                        p -= w;
                    } while (--n);
                }
                y--;
                i++;
            } while (y >= top);
        }
    }
    for (r = 0; r < (int)MB32(0xE0); r++)
        copy_u16((unsigned short *)(SCREEN + ((u32)r + MB32(0xD8)) * 2044u + MB32(0xD4) * 2u),
                 (const unsigned short *)(BUF + MB32(0xDC) * (u32)r), MB32(0xDC));
}
