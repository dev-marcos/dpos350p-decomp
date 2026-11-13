# SD/MMC controller driver, part 1 (helpers, clocks, data transfer).  exec'd by gen_platform.py
M = "SD/MMC"
MH = """
u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)
"""


def m(addr, size, name, desc, body):
    f(addr, size, name, M, desc, MH.strip("\n") + "\n\n" + body.strip("\n"))


m(0x400675A8, 4, "mmc_delay_us", "b delay_us.", """
void delay_us(u32 us);

void mmc_delay_us(u32 us) { delay_us(us); }
""")
m(0x400675AC, 4, "mmc_delay_ms", "b delay_ms.", """
void delay_ms(u32 ms);

void mmc_delay_ms(u32 ms) { delay_ms(ms); }
""")
m(0x4006763C, 60, "mmc_base", "Controller n register base: 0x01C0F000, 0x01C10000, 0x01C11000; 0 otherwise.", """
u32 mmc_base(u32 n)
{
    if (n == 0) return 0x01C0F000u;
    if (n == 1) return 0x01C10000u;
    if (n == 2) return 0x01C11000u;
    return 0;
}
""")
m(0x40067678, 68, "mmc_check_range", "start + count - 1 must not exceed the card capacity (sectors); else two messages and error 0xFFFCA9A3.", """
int mmc_check_range(u32 start, u32 count)
{
    u32 end = start + count - 1;
    if (mmc_capacity >= end)
        return 0;
    dbg_printf(STR(0x4008CC4C));
    dbg_printf(STR(0x4008CD70), mmc_capacity);
    return (int)0xFFFCA9A3u;
}
""")
m(0x400676BC, 128, "mmc_wait_rint", "Poll RINTSTS (+0x38) until a mask bit is set (5 us steps); an error bit (0xBFC2) or timeout gives a message and -0x98.", """
int mmc_wait_rint(u32 n, u32 timeout, u32 mask)
{
    u32 base = mmc_base(n), v;
    for (;;) {
        v = readl_a(base + 0x38);
        if (timeout-- == 0 || (v & 0xBFC2)) {
            dbg_printf(STR(0x4008D8AC), n, timeout, v & 0xBFC2);
            return -0x98;
        }
        mmc_delay_us(5);
        if (v & mask)
            return 0;
    }
}
""")
m(0x4006773C, 108, "mmc_update_clk", "CMD = 0x80202000 (update clock only, wait previous data), wait for the start bit to clear (200000 x 1 us).", """
int mmc_update_clk(u32 n)
{
    u32 base = mmc_base(n), t = 200000;
    writel_a(base + 0x18, 0x80202000u);
    while (readl_a(base + 0x18) & 0x80000000u) {
        if (t-- == 0) {
            dbg_printf(STR(0x4008F83C));
            return -1;
        }
        mmc_delay_us(1);
    }
    return 0;
}
""")
m(0x400677A8, 60, "mmc_periph0_freq", "PLL_PERIPH0 rate (copy in the SD source file): 24 MHz * N * K / 2.", """
s32 mmc_periph0_freq(void)
{
    u32 v = readl_a(0x01C20028u);
    s32 n = (s32)((v >> 8) & 0x1F) + 1, k = (s32)((v >> 4) & 3) + 1;
    s32 x = 24000000 * (s32)(short)(k * n);
    return (x + (s32)((u32)x >> 31)) >> 1;
}
""")
m(0x400677E4, 204, "mmc_set_mod_clk", "SDMMCn clock (CCU 0x01C20088 + 4n): OSC24M up to 24 MHz, else PLL_PERIPH0; M divider (<= 16) and N = 2^k (k < 4); sample/output phase by rate (400 kHz, 25 MHz, 50 MHz).", """
s32 mmc_periph0_freq(void);

int mmc_set_mod_clk(u32 n, u32 hz)
{
    u32 src, in, div, rem, nexp = 0, phase = 0;
    if (hz <= 24000000) { src = 0; in = 24000000; }
    else { src = 0x1000000; in = (u32)mmc_periph0_freq(); }
    div = in / hz;
    rem = in - hz * div;
    if (rem) div++;
    if (div >= 17) {
        do { div = (div + 1) >> 1; nexp++; } while (div >= 17);
        if (nexp >= 4) {
            dbg_printf(STR(0x4008D710), n, hz);
            return -1;
        }
    }
    if (hz >= 400001) {
        if (hz < 25000001) src |= 0x500000;
        else { src |= 0x400000; phase = hz < 50000001 ? 0x300 : 0x100; }
    }
    writel_a(0x01C20088u + (n << 2), (div - 1) | phase | src | (nexp << 16) | 0x80000000u);
    return 0;
}
""")
m(0x400678B0, 40, "mmc_clk_lowpower_on", "CLKCR (+0x04) |= bit 17; returns 0.", """
int mmc_clk_lowpower_on(u32 n)
{
    u32 r = mmc_base(n) + 4;
    writel_a(r, readl_a(r) | 0x20000);
    return 0;
}
""")
for a, name, op in ((0x400678D8, "mmc_clk_enable", "readl_a(r) | 0x10000"), (0x40067918, "mmc_clk_disable", "readl_a(r) & ~0x10000u")):
    m(a, 64, name, "CLKCR card clock (bit 16), then an update-clock command; -1 when that fails.", f"""
int {name}(u32 n)
{{
    u32 r = mmc_base(n) + 4;
    writel_a(r, {op});
    return mmc_update_clk(n) ? -1 : 0;
}}
""")
m(0x40067958, 36, "mmc_gctrl_clr_bit8", "GCTRL (+0x00) &= ~bit 8; returns 0.", """
int mmc_gctrl_clr_bit8(u32 n)
{
    u32 r = mmc_base(n);
    writel_a(r, readl_a(r) & ~0x100u);
    return 0;
}
""")
m(0x4006797C, 136, "mmc_config_clock", "Card clock off, module clock to hz, CLKCR divider 0, card clock on; message per failure (-1).", """
int mmc_clk_disable(u32 n);
int mmc_clk_enable(u32 n);
int mmc_set_mod_clk(u32 n, u32 hz);

int mmc_config_clock(u32 n, u32 hz)
{
    u32 base = mmc_base(n);
    if (mmc_clk_disable(n)) { dbg_printf(STR(0x4008E0A0)); return -1; }
    if (mmc_set_mod_clk(n, hz)) { dbg_printf(STR(0x4008E73C)); return -1; }
    writel_a(base + 4, readl_a(base + 4) & ~0xFFu);
    if (mmc_clk_enable(n)) { dbg_printf(STR(0x4008E754)); return -1; }
    dbg_printf(STR(0x4008E0BC));
    return 0;
}
""")
m(0x40067A04, 44, "mmc_reset", "GCTRL |= 7 (soft, FIFO, DMA reset), wait 1 ms; returns 0.", """
int mmc_reset(u32 n)
{
    u32 r = mmc_base(n);
    writel_a(r, readl_a(r) | 7);
    mmc_delay_us(1000);
    return 0;
}
""")
m(0x40067A30, 256, "mmc_trans_data_by_cpu", "PIO data phase: AHB access, then one 32-bit word per FIFO slot (wait while FIFO empty/full, timeout 1000 us per 256 bytes, at least 2 s).", """
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
""")
m(0x40067B30, 460, "mmc_trans_data_by_dma", "IDMAC: one descriptor per 4 KiB (max 256), flush buffer and descriptors, DMA on, IDMAC reset/enable, RX or TX interrupt, descriptor base, FIFO thresholds 0x20070008.", """
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
""")
m(0x40067CFC, 32, "mmc_set_bus_width", "CTYPE (+0x0C) = width code; returns 0.", """
int mmc_set_bus_width(u32 n, u32 ctype)
{
    writel_a(mmc_base(n) + 0x0C, ctype);
    return 0;
}
""")
m(0x40067D1C, 84, "mmc_idma_status", "IDST (+0x88) TX/RX interrupt bits; acknowledged (write 1) when set; returns them.", """
int mmc_idma_status(u32 n)
{
    u32 r = mmc_base(n) + 0x88, v = readl_a(r);
    int st = 0;
    if (v & 1) st = 1;
    if (v & 2) st |= 2;
    if (st > 0)
        writel_a(r, (u32)st | readl_a(r));
    return st;
}
""")
m(0x400688BC, 12, "dcache_clean_inv_mva", "Clean+invalidate the D-cache line and invalidate the I-cache line of an address (CP15 c7).", """
void dcache_clean_inv_mva(u32 addr)
{
    __asm__ volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r" (addr) : "memory");
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 1" : : "r" (addr) : "memory");
}
""")
m(0x400688C8, 52, "dcache_flush_range_se", "Every 32-byte line from start to end (inclusive, aligned down).", """
void dcache_clean_inv_mva(u32 addr);

void dcache_flush_range_se(u32 start, u32 end)
{
    u32 a = start & ~31u, e = end & ~31u;
    for (; a <= e; a += 32)
        dcache_clean_inv_mva(a);
}
""")
m(0x400688FC, 8, "dcache_flush_range", "b dcache_flush_range_se(addr, addr + len).", """
void dcache_flush_range_se(u32 start, u32 end);

void dcache_flush_range(u32 addr, u32 len) { dcache_flush_range_se(addr, addr + len); }
""")
