/*
 * persistence_read -- stock 0x40043620 (256 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads len bytes of the SPI flash at addr into buf. The command is 0x03 + 3 address bytes, or 0x13 + 4 address bytes when the word 0x42E68CD4 (4-byte addressing) is set. When the word 0x42E68CC4 is 1 the command is 0x3B / 0x3C instead; when it is positive a dummy byte follows the address and the word is cleared after the read (one-shot). Returns 0, or -1 when sys_spi_write_then_read failed.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"
#include "storage/settings.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
int sys_spi_write_then_read_raw(u32 dev, const volatile unsigned char *tx, u32 ntx, volatile unsigned char *rx, u32 nrx) __asm__("sys_spi_write_then_read");

#define READ_MODE (*(volatile int *)0x42E68CC4u)

int persistence_read(u32 dev, u32 addr, volatile unsigned char *buf, u32 len)
{
    volatile unsigned char cmd[8];
    int mode = READ_MODE, ret = 0;
    u32 n;
    if (*(volatile u32 *)0x42E68CD4u == 0) {
        cmd[0] = mode == 1 ? 0x3B : 0x03;
        cmd[1] = addr >> 16;
        cmd[2] = addr >> 8;
        cmd[3] = addr;
        n = 4;
    } else {
        cmd[0] = mode == 1 ? 0x3C : 0x13;
        cmd[1] = addr >> 24;
        cmd[2] = addr >> 16;
        cmd[3] = addr >> 8;
        cmd[4] = addr;
        n = 5;
    }
    if (mode > 0)
        cmd[n++] = 0;
    sys_spi_select(dev);
    if (sys_spi_write_then_read_raw(dev, cmd, n, buf, len) < 0)
        ret = -1;
    sys_spi_deselect(dev);
    if (READ_MODE > 0)
        READ_MODE = 0;
    return ret;
}
