/*
 * mmc_set_mod_clk -- stock 0x400677E4 (204 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SDMMCn clock (CCU 0x01C20088 + 4n): OSC24M up to 24 MHz, else PLL_PERIPH0; M divider (<= 16) and N = 2^k (k < 4); sample/output phase by rate (400 kHz, 25 MHz, 50 MHz).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"

#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_set_mod_clk(u32 n, u32 hz)
{
    u32 src, in, div, rem, nexp = 0, phase = 0;
    if (hz <= 24000000) { src = 0; in = 24000000; }
    else { src = 0x1000000; in = (u32)mmc_periph0_freq(); }
    div = in / hz;
    rem = in - hz * div;
    if (rem) div++;
    if (div >= 17) {
        do { div = (div + 1) >> 1; nexp++; } while (div >= 17);
        if (nexp >= 4) {
            dbg_printf(STR(0x4008D710), n, hz);
            return -1;
        }
    }
    if (hz >= 400001) {
        if (hz < 25000001) src |= 0x500000;
        else { src |= 0x400000; phase = hz < 50000001 ? 0x300 : 0x100; }
    }
    writel_a(0x01C20088u + (n << 2), (div - 1) | phase | src | (nexp << 16) | 0x80000000u);
    return 0;
}
