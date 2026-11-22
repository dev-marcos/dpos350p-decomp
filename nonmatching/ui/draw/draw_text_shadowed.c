/*
 * draw_text_shadowed -- stock 0x4002F0E8 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws s twice through 0x4002EE78: first at (x + dx, y + dy) (16 bit) in colour col << 24 (col and dy are the low bytes of the stack arguments), then at (x, y) in the colour word 0x42E68664 read on entry; 0x4002EA38 sets the colour.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void glyph_alpha_draw_raw(u32 x, u32 y, u32 s) __asm__("glyph_alpha_draw");

void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col)
{
    u32 saved = *(volatile u32 *)0x42E68664u;
    draw_colour_set((col & 0xFF) << 24);
    glyph_alpha_draw_raw((x + dx) & 0xFFFF, (y + (dy & 0xFF)) & 0xFFFF, s);
    draw_colour_set(saved);
    glyph_alpha_draw_raw(x, y, s);
}
