/*
 * spi_flash_page_program -- stock 0x4004372C (200 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Programs len bytes from buf at addr: write enable, wait, select, command 0x02 + 3 address bytes (or 0x12 + 4 when the word 0x42E68CD4 is set) followed by the data in one sys_spi_write_then_write, deselect, wait. Returns 0, or -1 when the write enable or the transfer failed.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

int spi_flash_page_program(u32 dev, u32 addr, const unsigned char *buf, int len)
{
    volatile unsigned char cmd[8];
    int ret = 0, n;
    if (*(volatile u32 *)0x42E68CD4u == 0) {
        cmd[0] = 0x02;
        cmd[1] = addr >> 16;
        cmd[2] = addr >> 8;
        cmd[3] = addr;
        n = 4;
    } else {
        cmd[0] = 0x12;
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
    if (sys_spi_write_then_write((int)dev, (const unsigned char *)cmd, n, buf, len) < 0)
        ret = -1;
    sys_spi_deselect(dev);
    spi_flash_wait_ready(dev);
    return ret;
}
