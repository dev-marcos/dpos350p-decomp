/*
 * trace_hit_zoom_ch2 -- stock 0x40069B04 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_hit_zoom(3).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void trace_hit_zoom_ch2(void)
{
    trace_hit_zoom(3);
}
