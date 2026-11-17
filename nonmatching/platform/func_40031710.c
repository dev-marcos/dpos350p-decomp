/*
 * colour_toggle_42e68d1a -- stock 0x40031710 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Toggles byte 0x42E68D1A and sets the colour 0x10262626 (was set) or 0x10303030 (was clear) through 0x4002EA38.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void draw_colour_set(u32 c);

void colour_toggle_42e68d1a(void)
{
    volatile unsigned char *p = (volatile unsigned char *)0x42E68D1Au;
    if (*p) {
        *p = 0;
        draw_colour_set(0x10262626u);
    } else {
        *p = 1;
        draw_colour_set(0x10303030u);
    }
}
