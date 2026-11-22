/*
 * battery_level_update -- stock 0x4004A2D0 (240 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Averaged LRADC code -> mV (float: code x slope + base + offset, truncated) -> battery level 0..10 in SYS_INFO+0x778 (thresholds 4150, 4080, 3970, 3900, 3840, 3790, 3760, 3730, 3690, 3620/3560 mV).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "power.h"

void battery_level_update(void)
{
    float t = (float)g_batt_avg;
    int mv, lv;
    t = g_batt_base + t * g_batt_slope;
    t = t + g_batt_offset;
    mv = (int)t;
    if (mv >= 4150) lv = 10;
    else if (mv >= 4080) lv = 9;
    else if (mv >= 3970) lv = 8;
    else if (mv >= 3900) lv = 7;
    else if (mv >= 3840) lv = 6;
    else if (mv >= 3790) lv = 5;
    else if (mv >= 3760) lv = 4;
    else if (mv >= 3730) lv = 3;
    else if (mv >= 3690) lv = 2;
    else if (mv >= 3620 || mv >= 3560) lv = 1;
    else lv = 0;
    SYS8(0x778) = lv;
}
