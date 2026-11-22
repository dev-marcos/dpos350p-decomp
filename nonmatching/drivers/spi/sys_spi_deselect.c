/*
 * sys_spi_deselect -- stock 0x40043088 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Chip select release: TCR SS bits, or the GPIO from the CS tables when spi_soft_cs.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/pio.h"
#include "drivers/spi.h"

void sys_spi_deselect(int spi)
{
    u32 base = SPI_BASE(spi);
    if (spi_soft_cs == 0) {
        writel_f(base + 8, (readl_f(base + 8) & ~0xB0u) | 0x80);
        return;
    }
    pio_set_bit((void *)spi_cs_port[spi_cs_index], spi_cs_pin[spi_cs_index]);
}
