/*
 * spi_flash_sector_erase -- stock 0x40043564 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Erases the 4 KB sector at addr: write enable, wait, then command 0x20 + 3 address bytes, or 0x21 + 4 address bytes when the word 0x42E68CD4 (4-byte addressing) is set; waits again. Returns 0, or -1 when the write enable failed or the transfer did not send every byte.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

int spi_flash_sector_erase(u32 dev, u32 addr)
{
    volatile unsigned char cmd[8];
    int ret = 0, n;
    if (*(volatile u32 *)0x42E68CD4u == 0) {
        cmd[0] = 0x20;
        cmd[1] = addr >> 16;
        cmd[2] = addr >> 8;
        cmd[3] = addr;
        n = 4;
    } else {
        cmd[0] = 0x21;
        cmd[1] = addr >> 24;
        cmd[2] = addr >> 16;
        cmd[3] = addr >> 8;
        cmd[4] = addr;
        n = 5;
    }
    if (spi_flash_write_enable(dev) < 0)
        ret = -1;
    spi_flash_wait_ready(dev);
    sys_spi_select(dev);
    if (sys_spi_transfer(dev, (const unsigned char *)cmd, 0, n) != n)
        ret = -1;
    sys_spi_deselect(dev);
    spi_flash_wait_ready(dev);
    return ret;
}
