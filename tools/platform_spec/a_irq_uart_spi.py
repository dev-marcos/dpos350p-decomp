# GIC/IRQ, UART + debug printf, SPI.  exec'd by gen_platform.py
G, U, S = "GIC/IRQ", "UART/printf", "SPI"

# ------------------------------------------------------------------ GIC / IRQ
f(0x400668D0, 8, "gic_readl", G, "Register read (copy in the GIC source file).", "u32 gic_readl(u32 addr) { return REG32(addr); }")
f(0x400668D8, 8, "gic_writel", G, "Register write.", "void gic_writel(u32 addr, u32 v) { REG32(addr) = v; }")
f(0x400668E0, 4, "default_irq_handler", G, "Empty handler installed in every slot of the IRQ table.", "void default_irq_handler(void) { }")
f(0x400668E4, 108, "irq_dispatch", G, "Read GICC_IAR (+0x200C), call the handler of (id - irq_base) when it is in range, then write the id to GICC_EOIR (+0x2010).", """
void irq_dispatch(void)
{
    u32 id = gic_readl(g_gic.base_lo + 0x200C) & 0x3FF;
    int n = (int)id - g_gic.irq_base;
    if (n < 0 || n >= g_gic.nr)
        return;
    irq_table[n]();
    gic_writel(g_gic.base_lo + 0x2010, id);
}
""")
for a, n, off, what in ((0x40066950, "gic_enable_irq", "0x1100", "ISENABLER"), (0x40066994, "gic_disable_irq", "0x1180", "ICENABLER")):
    f(a, 68, n, G, f"Set the bit of irq (+irq_base) in GICD_{what} (base + {off}); armcc's signed n/32 and n%32 sequences kept.", f"""
void {n}(gic_t *g, int irq)
{{
    int n = irq + g->irq_base;
    u32 bias = (u32)(n >> 4) >> 27;              /* 31 for negative n */
    u32 reg = g->base_lo + ((u32)((int)(n + bias) >> 5) << 2) + {off};
    gic_writel(reg, 1u << (n - (int)((n + bias) & 0xE0)));
}}
""")
f(0x400669D8, 8, "gic_cpu_id", G, "Always CPU 0.", "int gic_cpu_id(void) { return 0; }")
f(0x400669E0, 304, "gic_dist_init", G, "GIC distributor: off, target all SPIs to this CPU, level/edge config 0, priority 0xA0, disable all, on. Line count from GICD_TYPER (max 0x3FC).", """
void gic_dist_init(u32 base)
{
    u32 dist = base + 0x1000, lines, i, cpumask;
    gic_writel(dist, 0);
    lines = ((gic_readl(base + 0x1004) & 0x1F) + 1) * 32;
    if (lines > 0x3FC) lines = 0x3FC;
    cpumask = 1u << gic_cpu_id();
    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;
    for (i = 32; i < lines; i += 4)
        gic_writel(base + 0x1800 + i, cpumask);
    for (i = 32; i < lines; i += 16)
        gic_writel(base + 0x1C00 + i / 4, 0);
    for (i = 32; i < lines; i += 4)
        gic_writel(base + 0x1400 + i, 0xA0A0A0A0u);
    for (i = 32; i < lines; i += 32)
        gic_writel(base + 0x1180 + i / 8, 0xFFFFFFFFu);
    gic_writel(dist, 1);
}
""")
f(0x40066B10, 120, "gic_cpu_init", G, "Banked SGI/PPI enables and priorities, priority mask 0xF8, CPU interface on.", """
void gic_cpu_init(u32 base)
{
    int i;
    gic_writel(base + 0x1180, 0xFFFF0000u);
    gic_writel(base + 0x1100, 0x0000FFFFu);
    for (i = 0; i < 32; i += 4)
        gic_writel(base + 0x1400 + i, 0xA0A0A0A0u);
    gic_writel(base + 0x2004, 0xF8);
    gic_writel(base + 0x2000, 1);
}
""")
f(0x40066C6C, 20, "local_irq_enable_body", G, "CPSR_c = CPSR & 0x7F: clears the I bit (msr cpsr_c).", """
void local_irq_enable_body(void)
{
    u32 v;
    __asm__ volatile ("mrs %0, apsr" : "=r" (v));
    v &= 0x7F;
    __asm__ volatile ("msr cpsr_c, %0" : : "r" (v) : "memory");
}
""")
f(0x40066B88, 4, "local_irq_enable", G, "b local_irq_enable_body (stock name irq_enable).", """
void local_irq_enable_body(void);

void local_irq_enable(void) { local_irq_enable_body(); }
""")
f(0x40066B8C, 72, "gic_init", G, "GIC at 0x01C80000, irq_base 0, 0x9C table entries all set to default_irq_handler; distributor and CPU interface init. (Previously listed as irq_disable.)", """
void gic_init(void)
{
    int i;
    g_gic.base_lo = 0x01C80000u;
    g_gic.base_hi = 0;
    g_gic.irq_base = 0;
    g_gic.nr = 0x9C;
    for (i = 0; i < 0x9C; i++)
        irq_table[i] = default_irq_handler;
    gic_dist_init(g_gic.base_lo);
    gic_cpu_init(g_gic.base_lo);
}
""")
f(0x40066BD4, 136, "irq_register", G, "Install handler, set the priority byte (prio << 3) in GICD_IPRIORITYR, enable or disable the interrupt.", """
void irq_register(int irq, void (*handler)(void), int prio, int enable)
{
    u32 reg, v;
    int sh;
    dbg_printf(STR(0x4008E724));
    reg = g_gic.base_lo + (u32)((irq / 4) * 4) + 0x1400;
    sh = (irq - (irq / 4) * 4) * 8;
    v = gic_readl(reg);
    gic_writel(reg, (v & ~(0xFFu << sh)) | ((u32)prio << 3 << sh));
    irq_table[irq] = handler;
    if (enable)
        gic_enable_irq(&g_gic, irq);
    else
        gic_disable_irq(&g_gic, irq);
}
""")

# ------------------------------------------------------------------ UART / console printf
f(0x40066C80, 8, "readl_g", U, "Register read.", "u32 readl_g(u32 addr) { return REG32(addr); }")
f(0x40066C88, 8, "writel_g", U, "Register write.", "void writel_g(u32 addr, u32 v) { REG32(addr) = v; }")
f(0x40066C90, 96, "uart_putc", U, "Wait for TX FIFO not full (USR bit 1), write THR. Port 0/1/2 = 0x01C28000/400/800 (another port uses an unset register: OPEN, not tested).", """
void uart_putc(int port, int c)
{
    u32 base = 0;
    if (port == 0) base = 0x01C28000u;
    else if (port == 2) base = 0x01C28800u;
    else if ((u32)port < 2) base = 0x01C28400u;
    while (!(readl_g(base + 0x7C) & 2))
        ;
    writel_g(base, (u32)c);
}
""")
f(0x40066CF0, 20, "console_putc", U, "uart_putc on the debug port (g_uart_port, low byte sign-extended).",
  "void console_putc(int c) { uart_putc((signed char)g_uart_port, c); }")
f(0x40066D04, 36, "console_puts", U, "Write a NUL-terminated string.", """
void console_puts(const char *s)
{
    while (*s)
        console_putc((signed char)*s++);
}
""")
f(0x40066D28, 44, "console_putn", U, "Write c n times (n == 0: nothing).", """
void console_putn(int c, int n)
{
    if (n)
        do console_putc(c); while (--n);
}
""")
f(0x40066D54, 52, "console_put_reversed", U, "Write buf[n-1] .. buf[0].", """
void console_put_reversed(const char *buf, int n)
{
    const char *p;
    if (n < 1) return;
    p = buf + n - 1;
    do console_putc((signed char)*p--); while (--n);
}
""")
f(0x40066D88, 328, "console_print_num", U,
  "Integer to text: base < 0 = signed |base|; base 16 uses masks; base 10 inserts ',' every three digits; width < 0 = left aligned; width > 80 ignored; pad 0 means ' '.", """
void console_print_num(int val, int base, int width, int pad)
{
    char buf[40];
    int len = 0, left = 0, neg = 0;
    u32 v = (u32)val, d;
    if (!pad) pad = ' ';
    if (width < 0) { width = -width; left = 1; }
    if ((u32)width >= 0x51) width = 0;
    if (base < 0) {
        base = -base;
        if (val < 0) { neg = 1; v = (u32)-val; }
    }
    for (;;) {
        if (base == 16) {
            d = v & 0xF;
            v >>= 4;
        } else {
            /* the stock UDIV yields 0 for base 0 (format "%" + NUL); C division by 0 is undefined */
            u32 q = base ? v / (u32)base : 0;
            d = v - q * (u32)base;
            v = q;
        }
        buf[len++] = (char)(d + ((unsigned short)d < 10 ? '0' : '7'));
        if (v == 0) break;
        if (base == 10 && (len == 3 || len == 7 || len == 11 || len == 15))
            buf[len++] = ',';
    }
    if (neg) buf[len++] = '-';
    if (len >= width) {
        console_put_reversed(buf, len);
        return;
    }
    if (!left) console_putn(pad, width - len);
    console_put_reversed(buf, len);
    if (left) console_putn(pad, width - len);
}
""")
f(0x40066ED0, 232, "console_format_conv", U,
  "One conversion after '%': flags '0' (pad) and '-' (left), width digits, then c s d i u x X o '%'; any other character prints the argument in base 3. Returns the pointer after the conversion. The cases are the fragments 0x40066FB8..0x40067098.", """
const char *console_format_conv(const char *p, int arg)
{
    int width = 0, left = 0, base = 0, pad = ' ', c;
    unsigned int n;
    if (*p == '0') pad = '0';
    for (;;) {
        c = (signed char)*p++;
        if (c == 0) break;
        if ((unsigned)(c - '0') < 10) { width = width * 10 + c - '0'; continue; }
        if ((unsigned)c > 'x') { base = 3; break; }
        switch (c) {
        case '-': left = 1; continue;
        case '%': console_putc('%'); return p;
        case 'c':
            if (left) console_putc(arg & 0x7F);
            if (width > 0) console_putn(pad, width - 1);
            if (left) return p;
            console_putc(arg & 0x7F);
            return p;
        case 's':
            if (left) console_puts((const char *)arg);
            n = strlen((const char *)arg);
            if (n < (unsigned)width) console_putn(pad, width - (int)strlen((const char *)arg));
            if (left) return p;
            console_puts((const char *)arg);
            return p;
        case 'd': case 'i': base = -10; break;
        case 'u': base = 10; break;
        case 'X': case 'x': base = 16; break;
        case 'o': base = 8; break;
        default: base = 3; break;
        }
        break;
    }
    if (left) width = -width;
    console_print_num(arg, base, width, pad);
    return p;
}
""")
f(0x4006709C, 120, "dbg_printf", U, "printf to the debug UART when its bus clock gate is on (CCU 0x01C2006C bit 16 + port).", """
#include <stdarg.h>

void dbg_printf(const char *fmt, ...)
{
    va_list ap;
    const char *p = fmt;
    if (!((readl_g(0x01C2006Cu) >> (g_uart_port + 16)) & 1))
        return;
    va_start(ap, fmt);
    while (*p) {
        if (*p == '%')
            p = console_format_conv(p + 1, va_arg(ap, int));
        else
            console_putc((signed char)*p++);
    }
    va_end(ap);
}
""")
f(0x4006711C, 12, "debug_printf_stub", U, "Variadic no-op (the usbprint/debug macro target).", """
void debug_printf_stub(const char *fmt, ...) { (void)fmt; }
""")
f(0x40067128, 12, "serializer_set_cursor", U, "Store r0 at 0x4091F38C.", "void serializer_set_cursor(u32 v) { *(u32 *)0x4091F38Cu = v; }")

# ------------------------------------------------------------------ SPI
f(0x40042F00, 300, "sys_spi_init", S,
  "SPI clock divider (largest rate <= spi_max_hz, n < 10), master mode + soft reset (wait), TCR sample delay by rate and SS options, FIFO resets.", """
void sys_spi_init(int spi)
{
    u32 base = SPI_BASE(spi), v;
    int n = 0, rate = 0, clk = spi0_clk_init(spi);
    if (spi_no_ctrl == 0) {
        int maxhz = spi_max_hz;
        for (;;) {
            int nn = n + 1;
            rate = clk / (nn * 2);
            if (!(maxhz < rate)) break;
            n = nn;
            if (n >= 10) break;
        }
        dbg_printf(STR(0x4008E110), spi, rate, n);
        writel_f(base + 0x24, (u32)n | 0x1000);
    }
    v = (readl_f(base + 4) & ~4u) | 0x83 | 0x80000000u;
    if (spi_no_ctrl == 1) v &= ~2u;
    writel_f(base + 4, v);
    while (readl_f(base + 4) & 0x80000000u)
        ;
    if (spi_no_ctrl == 0) {
        v = readl_f(base + 8) & ~0x2800u;
        if (rate >= 80000000) v |= 0x800;
        else if (rate < 0x16E3601) v |= 0x2000;
        v = (v & ~3u) | 0x44;
        if (spi_soft_cs == 1) v &= ~0x40u;
        writel_f(base + 8, v);
    }
    writel_f(base + 0x18, readl_f(base + 0x18) | 0x8000 | 0x80000000u);
}
""")
for a, n, op, gpio, desc in ((0x4004302C, "sys_spi_select", "readl_f(base + 8) & ~0xB0u", "pio_clr_bit", "assert"),
                             (0x40043088, "sys_spi_deselect", "(readl_f(base + 8) & ~0xB0u) | 0x80", "pio_set_bit", "release")):
    f(a, 92 if a == 0x4004302C else 96, n, S, f"Chip select {desc}: TCR SS bits, or the GPIO from the CS tables when spi_soft_cs.", f"""
void {n}(int spi)
{{
    u32 base = SPI_BASE(spi);
    if (spi_soft_cs == 0) {{
        writel_f(base + 8, {op});
        return;
    }}
    {gpio}((void *)spi_cs_port[spi_cs_index], spi_cs_pin[spi_cs_index]);
}}
""")
f(0x400430E8, 80, "sys_spi_set_counts", S, "MBC = total, MTC = TX bytes, BCC = TX bytes | dual mode << 28 (24-bit counts).", """
void sys_spi_set_counts(int spi, u32 txn, u32 total)
{
    u32 base = SPI_BASE(spi);
    txn &= 0xFFFFFF;
    writel_f(base + 0x30, total & 0xFFFFFF);
    writel_f(base + 0x34, txn);
    writel_f(base + 0x38, txn | ((u32)spi_dual_mode << 28));
}
""")
f(0x40043138, 44, "sys_spi_start_xch", S, "TCR |= XCH (bit 31).", """
void sys_spi_start_xch(int spi)
{
    u32 base = SPI_BASE(spi);
    writel_f(base + 8, readl_f(base + 8) | 0x80000000u);
}
""")
f(0x40043164, 460, "sys_spi_cpu_transfer", S,
  "PIO transfer: start the burst, then feed the TX FIFO (8 words when >= 32 bytes, else bytes) and drain the RX FIFO (words to rx if not NULL; the byte tail always stores), wait for XCH to clear (10000 polls). Returns len, -1 on timeout.", """
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
""")
f(0x40043330, 48, "sys_spi_transfer", S, "Length check (< 16 MiB, else message and -1); returns len whatever the transfer result.", """
int sys_spi_transfer(int spi, const unsigned char *tx, unsigned char *rx, int len)
{
    if (len >= 0x1000000) {
        dbg_printf(STR(0x4008D20C), len);
        return -1;
    }
    sys_spi_cpu_transfer(spi, tx, rx, len);
    return len;
}
""")
f(0x40043360, 104, "sys_spi_write_then_read", S, "Send txlen bytes, set the dual-mode bits 7:4 if dual mode is on, read rxlen bytes. 0 on success, -1 otherwise.", """
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
""")
f(0x400433C8, 84, "sys_spi_write_then_write", S, "Two transmit-only transfers. 0 on success, -1 otherwise.", """
int sys_spi_write_then_write(int spi, const unsigned char *tx0, int len0, const unsigned char *tx1, int len1)
{
    if (sys_spi_transfer(spi, tx0, 0, len0) != len0)
        return -1;
    if (sys_spi_transfer(spi, tx1, 0, len1) != len1)
        return -1;
    return 0;
}
""")
