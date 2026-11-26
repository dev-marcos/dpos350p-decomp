/*
 * draw_state_defaults -- stock 0x4002DC4C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * glyph14_table_build, 0x4002CCD8, text_centring_set(0), text_spacing_set(1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"

void draw_state_defaults(void)
{
    glyph14_table_build();
    fonts_init();
    text_centring_set(0);
    text_spacing_set(1);
}
