/*
 * sys_spi_select -- stock 0x4004302C (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Chip select assert: TCR SS bits, or the GPIO from the CS tables when spi_soft_cs.
 */
#include "hw/platform.h"

void sys_spi_select(int spi)
{
    u32 base = SPI_BASE(spi);
    if (spi_soft_cs == 0) {
        writel_f(base + 8, readl_f(base + 8) & ~0xB0u);
        return;
    }
    pio_clr_bit((void *)spi_cs_port[spi_cs_index], spi_cs_pin[spi_cs_index]);
}
