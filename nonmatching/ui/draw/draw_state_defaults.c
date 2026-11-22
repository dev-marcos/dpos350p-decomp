/*
 * draw_state_defaults -- stock 0x4002DC4C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * table_scatter_42cf64b4, 0x4002CCD8, byte_42e68be0_set(0), half_42e68be2_set(1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"

void draw_state_defaults(void)
{
    table_scatter_42cf64b4();
    fonts_init();
    byte_42e68be0_set(0);
    half_42e68be2_set(1);
}
