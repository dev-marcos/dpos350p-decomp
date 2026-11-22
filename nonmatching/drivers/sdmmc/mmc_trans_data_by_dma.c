/*
 * mmc_trans_data_by_dma -- stock 0x40067B30 (460 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * IDMAC: one descriptor per 4 KiB (max 256), flush buffer and descriptors, DMA on, IDMAC reset/enable, RX or TX interrupt, descriptor base, FIFO thresholds 0x20070008.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

typedef struct { u32 *buf; unsigned char flags; unsigned char pad[3]; u32 blksz, blocks; } mmc_data_t;
void *memset_w(void *d, int v, unsigned int n);
void dcache_flush_range(u32 addr, u32 len);

int mmc_trans_data_by_dma(u32 n, mmc_data_t *d, u32 *desc)
{
    u32 bytes = d->blksz * d->blocks, base, last, cnt, buf, i, count = 0;
    u32 *p = desc;
    base = mmc_base(n);
    cnt = bytes >> 12;
    last = bytes & 0xFFF;
    if (last) cnt++;
    else last = 0x1000;
    buf = (u32)d->buf;
    if (cnt >= 0x101)
        return -1;
    dcache_flush_range(buf, bytes);
    for (i = 0; i < cnt; i++) {
        u32 d0, d1;
        memset_w(p, 0, 16);
        d0 = p[0] | 0x80000012u;
        p[0] = d0;
        if (cnt <= 1 || i + 1 == cnt)
            d1 = ((p[1] >> 16) << 16) | last;
        else
            d1 = (p[1] & 0xFFFF0000u) | 0x1000;
        p[1] = d1;
        p[2] = buf;
        if (i == 0)
            p[0] = d0 | 8;
        if (i + 1 == cnt) {
            p[0] = (p[0] & ~2u) | 0x24;
            p[3] = 0;
        } else {
            p[3] = (u32)(p + 4);
        }
        count++;
        buf += 0x1000;
        p += 4;
    }
    dcache_flush_range((u32)desc, (count + 1) << 4);
    writel_a(base, readl_a(base) & ~0x80000000u);
    writel_a(base, readl_a(base) | 0x24);
    writel_a(base + 0x80, 1);
    while (readl_a(base + 0x80) & 1)
        ;
    writel_a(base + 0x80, 0x82);
    writel_a(base + 0x8C, (readl_a(base + 0x8C) & ~3u) | ((d->flags & 2) ? 1 : 2));
    writel_a(base + 0x84, (u32)desc);
    writel_a(base + 0x40, 0x20070008u);
    return 0;
}
