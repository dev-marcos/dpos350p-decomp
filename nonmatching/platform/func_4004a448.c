/*
 * battery_service -- stock 0x4004A448 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Charger status; LRADC readings are summed, and about once a second (>= 1001 ms, >= 3 readings) averaged into the battery level, then the empty-battery and auto power-off checks run.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void battery_service(void)
{
    charge_status_read();
    if (g_tick_count - g_batt_last >= 1001 && g_batt_count >= 3) {
        g_batt_avg = g_batt_sum / g_batt_count;
        g_batt_last = g_tick_count;
        g_batt_sum = 0;
        g_batt_count = 0;
        battery_level_update();
        battery_empty_check();
        auto_off_check();
        return;
    }
    g_batt_sum = func_40068e74() + g_batt_sum;
    g_batt_count = g_batt_count + 1;
}
