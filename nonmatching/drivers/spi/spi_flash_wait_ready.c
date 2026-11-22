/*
 * spi_flash_wait_ready -- stock 0x400434F4 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Polls the flash status register (command 0x05) until the busy bit is clear.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

void spi_flash_wait_ready(u32 dev)
{
    while (spi_flash_cmd_read1(dev, 5) & 1)
        ;
}
