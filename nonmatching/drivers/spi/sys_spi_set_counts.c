/*
 * sys_spi_set_counts -- stock 0x400430E8 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MBC = total, MTC = TX bytes, BCC = TX bytes | dual mode << 28 (24-bit counts).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/spi.h"

void sys_spi_set_counts(int spi, u32 txn, u32 total)
{
    u32 base = SPI_BASE(spi);
    txn &= 0xFFFFFF;
    writel_f(base + 0x30, total & 0xFFFFFF);
    writel_f(base + 0x34, txn);
    writel_f(base + 0x38, txn | ((u32)spi_dual_mode << 28));
}
