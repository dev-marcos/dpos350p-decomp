/*
 * trace_hit_ch1 -- stock 0x40069AEC (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_hit_main(0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void trace_hit_ch1(void)
{
    trace_hit_main(0);
}
