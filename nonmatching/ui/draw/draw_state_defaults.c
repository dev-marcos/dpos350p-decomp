/*
 * draw_state_defaults -- stock 0x4002DC4C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * table_scatter_42cf64b4, 0x4002CCD8, byte_42e68be0_set(0), half_42e68be2_set(1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void table_scatter_42cf64b4(void);
void fonts_init(void);
void byte_42e68be0_set(u32 a);
void half_42e68be2_set(u32 a);

void draw_state_defaults(void)
{
    table_scatter_42cf64b4();
    fonts_init();
    byte_42e68be0_set(0);
    half_42e68be2_set(1);
}
