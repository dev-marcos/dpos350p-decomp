/*
 * sys_spi_init -- stock 0x40042F00 (300 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SPI clock divider (largest rate <= spi_max_hz, n < 10), master mode + soft reset (wait), TCR sample delay by rate and SS options, FIFO resets.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/spi.h"
#include "drivers/uart.h"

void sys_spi_init(int spi)
{
    u32 base = SPI_BASE(spi), v;
    int n = 0, rate = 0, clk = spi0_clk_init(spi);
    if (spi_no_ctrl == 0) {
        int maxhz = spi_max_hz;
        for (;;) {
            int nn = n + 1;
            rate = clk / (nn * 2);
            if (!(maxhz < rate)) break;
            n = nn;
            if (n >= 10) break;
        }
        dbg_printf(STR(0x4008E110), spi, rate, n);
        writel_f(base + 0x24, (u32)n | 0x1000);
    }
    v = (readl_f(base + 4) & ~4u) | 0x83 | 0x80000000u;
    if (spi_no_ctrl == 1) v &= ~2u;
    writel_f(base + 4, v);
    while (readl_f(base + 4) & 0x80000000u)
        ;
    if (spi_no_ctrl == 0) {
        v = readl_f(base + 8) & ~0x2800u;
        if (rate >= 80000000) v |= 0x800;
        else if (rate < 0x16E3601) v |= 0x2000;
        v = (v & ~3u) | 0x44;
        if (spi_soft_cs == 1) v &= ~0x40u;
        writel_f(base + 8, v);
    }
    writel_f(base + 0x18, readl_f(base + 0x18) | 0x8000 | 0x80000000u);
}
