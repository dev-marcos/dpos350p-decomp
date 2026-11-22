/*
 * sys_spi_start_xch -- stock 0x40043138 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * TCR |= XCH (bit 31).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/spi.h"

void sys_spi_start_xch(int spi)
{
    u32 base = SPI_BASE(spi);
    writel_f(base + 8, readl_f(base + 8) | 0x80000000u);
}
