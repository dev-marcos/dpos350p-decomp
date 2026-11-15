/*
 * power_monitor_init -- stock 0x4004A198 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * LRADC (battery) set-up, auto power-off deadline, default battery coefficients.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void power_monitor_init(void)
{
    lradc_init();
    auto_off_arm();
    battery_coeffs_default();
}
