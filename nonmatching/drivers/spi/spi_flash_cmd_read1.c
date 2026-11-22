/*
 * spi_flash_cmd_read1 -- stock 0x40043494 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sends a one-byte command and reads one byte back (chip select around it).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
int  sys_spi_write_then_read_raw(u32 dev, const volatile unsigned char *tx, u32 ntx, volatile unsigned char *rx, u32 nrx) __asm__("sys_spi_write_then_read");

u32 spi_flash_cmd_read1(u32 dev, u32 cmd)
{
    volatile unsigned char c = cmd, rx;
    sys_spi_select(dev);
    sys_spi_write_then_read_raw(dev, &c, 1, &rx, 1);
    sys_spi_deselect(dev);
    return rx;
}
