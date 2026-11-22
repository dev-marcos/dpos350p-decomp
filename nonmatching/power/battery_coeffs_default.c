/*
 * battery_coeffs_default -- stock 0x4004A2A0 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Default battery conversion: offset 170.0, slope 32.947, base 2565.74 (float; mV = slope x LRADC + base + offset).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "power.h"

void battery_coeffs_default(void)
{
    *(volatile u32 *)0x42E68C9Cu = 0x432A0000u;   /* 170.0f */
    *(volatile u32 *)0x42E68C94u = 0x4203CA1Bu;   /* 32.947f */
    *(volatile u32 *)0x42E68C98u = 0x45205BCAu;   /* 2565.74f */
}
