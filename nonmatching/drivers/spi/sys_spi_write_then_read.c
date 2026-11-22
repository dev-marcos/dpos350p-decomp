/*
 * sys_spi_write_then_read -- stock 0x40043360 (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Send txlen bytes, set the dual-mode bits 7:4 if dual mode is on, read rxlen bytes. 0 on success, -1 otherwise.
 */
#include "hw/platform.h"
#include "drivers/spi.h"

int sys_spi_write_then_read(int spi, const unsigned char *tx, int txlen, unsigned char *rx, int rxlen)
{
    if (sys_spi_transfer(spi, tx, 0, txlen) != txlen)
        return -1;
    if (spi_dual_mode > 0)
        spi_dual_mode |= 0xF0;
    if (sys_spi_transfer(spi, 0, rx, rxlen) != rxlen)
        return -1;
    return 0;
}
