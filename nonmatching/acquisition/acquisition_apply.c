/*
 * acquisition_apply -- stock 0x4003A654 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * timebase_slow_flag_update, sample_divisor_setup, 0x4003A984, fpga_sample_count_write(SYS+0x30), then timebase_apply.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"

void acquisition_apply(void)
{
    timebase_slow_flag_update();
    sample_divisor_setup();
    acquisition_timing_setup();
    fpga_sample_count_write(SYS32(0x30));
    timebase_apply();
}
