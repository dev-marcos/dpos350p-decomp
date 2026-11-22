/*
 * sys_spi_cpu_transfer -- stock 0x40043164 (460 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PIO transfer: start the burst, then feed the TX FIFO (8 words when >= 32 bytes, else bytes) and drain the RX FIFO (words to rx if not NULL; the byte tail always stores), wait for XCH to clear (10000 polls). Returns len, -1 on timeout.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/spi.h"
#include "drivers/uart.h"

int sys_spi_cpu_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len)
{
    u32 base = SPI_BASE(spi);
    int txn = tx ? len : 0, rxn = len, i, t;
    unsigned char *rxp = rx;
    if (spi_no_ctrl == 0) {
        sys_spi_set_counts(spi, (u32)txn, (u32)len);
        sys_spi_start_xch(spi);
    }
    while (rxn | txn) {
        if (txn > 0) {
            if (txn >= 32) {
                if (((readl_f(base + 0x1C) >> 16) & 0xFF) < 32) {
                    const u32 *w = (const u32 *)tx;
                    for (i = 0; i < 8; i++)
                        writel_f(base + 0x200, *w++);
                    txn -= 32;
                    tx += 32;
                }
            } else if (((readl_f(base + 0x1C) >> 16) & 0xFF) < (u32)(64 - txn)) {
                const unsigned char *b = tx;
                do writeb_f(base + 0x200, *b++); while (--txn);
            }
        }
        if (rxn <= 0)
            continue;
        if (rxn >= 32) {
            if ((readl_f(base + 0x1C) & 0xFF) >= 32) {
                for (i = 0; i < 8; i++) {
                    u32 w = readl_f(base + 0x300);
                    if (rxp) { *(u32 *)rxp = w; rxp += 4; }
                }
                rxn -= 32;
            }
        } else if ((readl_f(base + 0x1C) & 0xFF) >= (u32)rxn) {
            unsigned char *b = rxp;
            do *b++ = (unsigned char)readb_f(base + 0x300); while (--rxn);
        }
    }
    if (spi_no_ctrl != 0)
        return len;
    t = 10000;
    while (readl_f(base + 8) & 0x80000000u)
        if (--t <= 0) break;
    if (t == 0) {
        dbg_printf(STR(0x4008D93C));
        return -1;
    }
    return len;
}
