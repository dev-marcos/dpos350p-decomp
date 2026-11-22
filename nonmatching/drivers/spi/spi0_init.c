/*
 * spi0_init -- stock 0x40043474 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SPI0 pins, clock gate, then sys_spi_init(mode).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

void spi0_init(u32 mode)
{
    spi0_pins_init(mode);
    spi0_clock_on();
    sys_spi_init(mode);
}
