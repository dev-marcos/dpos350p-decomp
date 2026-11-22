/*
 * sys_spi_write_then_write -- stock 0x400433C8 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Two transmit-only transfers. 0 on success, -1 otherwise.
 */
#include "hw/platform.h"

int sys_spi_write_then_write(int spi, const unsigned char *tx0, int len0, const unsigned char *tx1, int len1)
{
    if (sys_spi_transfer(spi, tx0, 0, len0) != len0)
        return -1;
    if (sys_spi_transfer(spi, tx1, 0, len1) != len1)
        return -1;
    return 0;
}
