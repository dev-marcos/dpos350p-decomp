/*
 * mmc_trans_data_by_cpu -- stock 0x40067A30 (256 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PIO data phase: AHB access, then one 32-bit word per FIFO slot (wait while FIFO empty/full, timeout 1000 us per 256 bytes, at least 2 s).
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"
#include "shared/drivers.h"

typedef struct { u32 *buf; unsigned char flags; unsigned char pad[3]; u32 blksz, blocks; } mmc_data_t;

int mmc_trans_data_by_cpu(u32 n, mmc_data_t *d)
{
    u32 *buf = d->buf, rd = d->flags & 1, busy = rd ? 4 : 8;
    u32 bytes = d->blksz * d->blocks, t = 1000 * (bytes >> 8), base, words, i;
    base = mmc_base(n);
    if (t < 2000000) t = 2000000;
    writel_a(base, readl_a(base) | 0x80000000u);
    words = bytes >> 2;
    for (i = 0; i < words; i++) {
        while (readl_a(base + 0x3C) & busy) {
            if (t-- == 0) {
                dbg_printf(STR(0x4008D8D0), i, readl_a(base + 0x3C));
                return -1;
            }
            mmc_delay_us(1);
        }
        if (rd) *buf = readl_a(base + 0x200);
        else writel_a(base + 0x200, *buf);
        buf++;
    }
    return 0;
}
