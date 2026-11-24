/*
 * level_columns_draw -- stock 0x40062C98 (352 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Level-column graph (enabled by byte 0x42E68669, skipped while 0x42E68BC7 is set): for each of the M+0xDC columns and each step i from the height h = M+0xE0 down to the column top (s16 0x42E60008 + 2col), the colour of table 0x42E56C30[(int)(i * 1000.0 / h)] is set and M+0x184 pixels of the column are painted upwards from the bottom row of the buffer 0x42B8C194 (width M+0xDC; the start does not move with i, as in the stock code); then the M+0xE0 rows are copied to the screen *0x40095498 at row M+0xD8 + r (stride 1022 pixels), column M+0xD4. M = 0x42E68668.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "util.h"
#include "state/levels_block.h"

#define COLOUR16 (*(volatile unsigned short *)0x42E68660u)
#define SCREEN (*(volatile u32 *)0x40095498u)

#define TOP16(c) (*(volatile short *)(0x42E60008u + 2 * (c)))
#define TBL(i) (*(volatile u32 *)(0x42E56C30u + 4 * (i)))
#define BUF ((volatile unsigned short *)0x42B8C194u)

void level_columns_draw(void)
{
    u32 h0 = LEVELS(panel_a_rows);
    double k;
    int col, r;
    if (g_touch->serviced)
        return;
    if (LEVELS(panel_a_on) == 0)
        return;
    k = 1000.0 / (double)h0;
    for (col = 0; col < (int)LEVELS(f_0dc); col++) {
        int top = TOP16(col), y = (int)h0, i = 0;
        if (y >= top) {
            do {
                u32 w;
                int n;
                volatile unsigned short *p;
                draw_colour_set(TBL((int)((double)i * k)));
                w = LEVELS(f_0dc);
                p = BUF + (w * LEVELS(panel_a_rows) + (u32)col);
                n = (int)LEVELS(f_184);
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
    for (r = 0; r < (int)LEVELS(panel_a_rows); r++)
        copy_u16((unsigned short *)(SCREEN + ((u32)r + LEVELS(panel_a_y)) * 2044u + LEVELS(panel_a_x) * 2u),
                 (const unsigned short *)(BUF + LEVELS(f_0dc) * (u32)r), LEVELS(f_0dc));
}
