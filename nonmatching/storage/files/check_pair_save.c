/*
 * check_pair_save -- stock 0x40064DC0 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40063A50; r = 0x40064C28() & 0x40064CF0() & 1; settings_a_save when r; 0x4003EB40; byte 0x42E68AE0+0x45 = r.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "selftest.h"
#include "storage/files.h"
#include "storage/settings.h"
#include "system.h"

void check_pair_save(void)
{
    u32 a, r;
    table_684_init();
    a = adc_phase_trim8_check();
    r = adc_phase_trim4x2_check() & a & 1;
    if (r)
        settings_a_save();
    acq_defaults_reset();
    *(volatile unsigned char *)0x42E68B25u = r;
}
