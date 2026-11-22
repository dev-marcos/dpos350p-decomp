/*
 * sys_spi_transfer -- stock 0x40043330 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Length check (< 16 MiB, else message and -1); returns len whatever the transfer result.
 */
#include "hw/platform.h"
#include "drivers/spi.h"
#include "drivers/uart.h"

int sys_spi_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len)
{
    if (len >= 0x1000000) {
        dbg_printf(STR(0x4008D20C), len);
        return -1;
    }
    sys_spi_cpu_transfer(spi, tx, rx, len);
    return len;
}
