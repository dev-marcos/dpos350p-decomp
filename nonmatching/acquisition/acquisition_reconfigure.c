/*
 * acquisition_reconfigure -- stock 0x4003A62C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * sample_divisor_setup, 0x4003A680, 0x4005DB9C, sine_table_build (0x4005D27C; its first sine argument is whatever d0 holds here), acquisition_geometry_setup, then runtime_state_commit_copy.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "generator.h"
#include "ui/draw.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void sine_table_build_raw(void) __asm__("sine_table_build");

void acquisition_reconfigure(void)
{
    sample_divisor_setup();
    timebase_step_table_init();
    intensity_palettes_init();
    sine_table_build_raw();
    acquisition_geometry_setup();
    runtime_state_commit_copy();
}
