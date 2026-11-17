/*
 * display_start -- stock 0x400344E0 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * draw_state_defaults, colour 0x00FFFFFF, word_42e68c74_set(0x42E64A98), lcd_start, fb_clear_all.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_state_defaults(void);
void func_4002ea38(u32 c);
void word_42e68c74_set(u32 a);
void lcd_start(void);
void fb_clear_all(void);

void display_start(void)
{
    draw_state_defaults();
    func_4002ea38(0x00FFFFFFu);
    word_42e68c74_set(0x42E64A98u);
    lcd_start();
    fb_clear_all();
}
