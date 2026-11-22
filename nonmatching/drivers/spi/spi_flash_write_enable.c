/*
 * spi_flash_write_enable -- stock 0x40043514 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Write enable (command 0x06); returns 0 when the transfer reports 1 byte, else -1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_spi_select(u32 dev);
void sys_spi_deselect(u32 dev);
int  sys_spi_write_then_read(u32 dev, const volatile unsigned char *tx, u32 ntx, volatile unsigned char *rx, u32 nrx);

int spi_flash_write_enable(u32 dev)
{
    volatile unsigned char c = 6;
    int r;
    sys_spi_select(dev);
    r = sys_spi_transfer(dev, (const unsigned char *)&c, 0, 1) == 1 ? 0 : -1;
    sys_spi_deselect(dev);
    return r;
}
