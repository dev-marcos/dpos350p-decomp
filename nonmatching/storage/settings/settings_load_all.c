/*
 * settings_load_all -- stock 0x40044794 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Start-up load of every record: E (rebuilds defaults without the magic), F, the current slot, B, calibration, A, G, D, then C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

void settings_load_all(void)
{
    settings_e_load();
    settings_f_load();
    settings_slot_load();
    settings_b_load();
    calibration_load_wrapper();
    settings_a_load();
    settings_g_load();
    settings_d_load();
    settings_c_load();
}
