/*
 * spi_flash_write_enable -- stock 0x40043514 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Write enable (command 0x06); returns 0 when the transfer reports 1 byte, else -1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

int spi_flash_write_enable(u32 dev)
{
    volatile unsigned char c = 6;
    int r;
    sys_spi_select(dev);
    r = sys_spi_transfer(dev, (const unsigned char *)&c, 0, 1) == 1 ? 0 : -1;
    sys_spi_deselect(dev);
    return r;
}
