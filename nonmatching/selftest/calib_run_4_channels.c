/*
 * calib_run_4_channels -- stock 0x40060D68 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Block 0x42E68668: +0x190 = 1, +5 = 1; for i = 0..3: +0xF4 = i, then 0x400612FC, 0x40060FE8, 0x40060DC8, 0x40060BA4; then +0x190 = 0, +0xF4 = its old low byte, and settings_c_save.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "selftest.h"
#include "storage/settings.h"
#include "state/levels_block.h"


void calib_run_4_channels(void)
{
    u32 old, i;
    LEVELS(f_190) = 1;
    old = LEVELS(spur_mode);
    LEVELS(f_005) = 1;
    for (i = 0; i < 4; i++) {
        LEVELS(spur_mode) = i;
        levels_view_layout();
        auto_level_search();
        fft_spur_cancel();
        fft_spur_capture();
    }
    LEVELS(f_190) = 0;
    LEVELS(spur_mode) = old & 0xFF;
    settings_c_save();
}
