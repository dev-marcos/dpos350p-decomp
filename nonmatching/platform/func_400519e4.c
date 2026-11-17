/*
 * touch_targets_test -- stock 0x400519E4 (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch test screen (DERIVED from what it draws): clears the touch state, draws five grey (0xCCCCCC) 80 x 80 squares at the four corners (20, 20), (924, 20), (20, 500), (924, 500) and the centre (472, 260), then polls the touch controller until each square has been touched (state byte 0x42E68BBC = 2 with x, y at 0x42E68BB4 / 0x42E68BB6 inside 21..99 of the square), redrawing a touched square in 0x88FF. Then clears the screen and byte 0x42E68B26.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void touch_state_clear(void);
void fb_target_back(void);
void fb_clear_current(void);
void draw_colour_set(u32 c);
void word_42e68c74_set(u32 v);
void half_42e68be2_set(u32 v);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void fb_present(void);
void gt911_poll(void);
#define TS8(o) (*(volatile unsigned char *)(0x42E68BB0u + (o)))
#define TS16(o) (*(volatile short *)(0x42E68BB0u + (o)))
#define IN(v, lo) ((u32)((v) - (lo)) < 0x4Fu)

void touch_targets_test(void)
{
    u32 got = 0;
    touch_state_clear();
    fb_target_back();
    fb_clear_current();
    draw_colour_set(0xCCCCCC);
    word_42e68c74_set(0x42E658D8);
    half_42e68be2_set(1);
    rect_fill(0x14, 0x14, 0x50, 0x50);
    rect_fill(0x39C, 0x14, 0x50, 0x50);
    rect_fill(0x14, 0x1F4, 0x50, 0x50);
    rect_fill(0x39C, 0x1F4, 0x50, 0x50);
    rect_fill(0x1D8, 0x104, 0x50, 0x50);
    fb_present();
    do {
        gt911_poll();
        if (TS8(0xC) == 2) {
            int x = TS16(4);
            u32 bit = 0, rx = 0, ry = 0;
            if (IN(x, 0x15) && IN(TS16(6), 0x15))
                bit = 1, rx = 0x14, ry = 0x14;
            else if (IN(x, 0x39D) && IN(TS16(6), 0x15))
                bit = 2, rx = 0x39C, ry = 0x14;
            else if (IN(x, 0x15) && IN(TS16(6), 0x1F5))
                bit = 4, rx = 0x14, ry = 0x1F4;
            else if (IN(x, 0x39D) && IN(TS16(6), 0x1F5))
                bit = 8, rx = 0x39C, ry = 0x1F4;
            else if (IN(x, 0x1D9) && IN(TS16(6), 0x105))
                bit = 0x10, rx = 0x1D8, ry = 0x104;
            if (bit) {
                got |= bit;
                draw_colour_set(0x88FF);
                rect_fill(rx, ry, 0x50, 0x50);
            }
        }
        fb_present();
    } while ((got & 0xFF) != 0x1F);
    fb_clear_current();
    *(volatile unsigned char *)(0x42E68AE0u + 0x46) = 0;
}
