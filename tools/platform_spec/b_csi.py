# CSI0 driver (Allwinner csi_reg layout).  exec'd by gen_platform.py
C = "CSI"
CSIH = """
#define csi_base_tbl ((u32 *)0x42E68C50u)
u32 csi_readl(u32 addr);
void csi_writel(u32 addr, u32 v);
void csi_setbits(u32 addr, u32 bits);
void csi_clrbits(u32 addr, u32 bits);
void csi_clrsetbits(u32 addr, u32 clr, u32 set);
void dcache_clean_inv_all(void);
void csi_writel_t(u32 addr, u32 v);
u32 csi_readl_t(u32 addr);
"""


def c(addr, size, name, desc, body):
    f(addr, size, name, C, desc, CSIH.strip("\n") + "\n\n" + body.strip("\n"))


c(0x4006582C, 8, "csi_readl", "Register read.", "u32 csi_readl(u32 addr) { return REG32(addr); }")
c(0x40065834, 8, "csi_writel", "Register write.", "void csi_writel(u32 addr, u32 v) { REG32(addr) = v; }")
c(0x4006583C, 40, "csi_setbits", "Read, OR, write.", "void csi_setbits(u32 addr, u32 bits) { csi_writel(addr, bits | csi_readl(addr)); }")
c(0x40065864, 40, "csi_clrbits", "Read, AND NOT, write.", "void csi_clrbits(u32 addr, u32 bits) { csi_writel(addr, csi_readl(addr) & ~bits); }")
c(0x4006588C, 4, "csi_writel_t", "b csi_writel (second entry used by tail calls).", "void csi_writel_t(u32 addr, u32 v) { csi_writel(addr, v); }")
c(0x40065890, 4, "csi_readl_t", "b csi_readl.", "u32 csi_readl_t(u32 addr) { return csi_readl(addr); }")
c(0x40065894, 40, "csi_clrsetbits", "(v & ~clr) | set.",
  "void csi_clrsetbits(u32 addr, u32 clr, u32 set) { csi_writel(addr, set | (csi_readl(addr) & ~clr)); }")
c(0x400658BC, 32, "csi_set_base_addr", "Store the register base of CSI sel (0 or 1); -1 otherwise.", """
int csi_set_base_addr(u32 sel, u32 addr)
{
    if (sel >= 2) return -1;
    csi_base_tbl[sel] = addr;
    return 0;
}
""")
c(0x400658DC, 16, "csi_enable", "CSI_EN bit 0 on.", "void csi_enable(u32 sel) { csi_setbits(csi_base_tbl[sel], 1); }")
c(0x400658EC, 16, "csi_disable", "CSI_EN bit 0 off.", "void csi_disable(u32 sel) { csi_clrbits(csi_base_tbl[sel], 1); }")
c(0x400658FC, 88, "csi_if_cfg", "IF_CFG (+0x04) from {src_type (bit 21), data_width (9:8), interface (4:0)} as signed bytes.", """
void csi_if_cfg(u32 sel, const signed char *cfg)
{
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x200000, (u32)(int)cfg[0] << 21);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x1F, (u32)(int)cfg[2]);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x300, (u32)(int)cfg[1] << 8);
}
""")
c(0x40065954, 124, "csi_timing_cfg", "IF_CFG polarity bits: vref (18), href (17), clock edge (16, set when cfg[3] == 1), field (19).", """
void csi_timing_cfg(u32 sel, const signed char *cfg)
{
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x40000, (u32)(int)cfg[1] << 18);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x20000, (u32)(int)cfg[2] << 17);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x10000, cfg[3] == 1 ? 0x10000 : 0);
    csi_clrsetbits(csi_base_tbl[sel] + 4, 0x80000, (u32)(int)cfg[0] << 19);
}
""")
c(0x400659D0, 144, "csi_fmt_cfg", "Channel CH_CFG (+0x44 + ch*0x100): input format (23:20), output format (19:16), field select (11:10), input seq (9:8).", """
void csi_fmt_cfg(u32 sel, u32 ch, const signed char *fmt)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xF00000, (u32)(int)fmt[0] << 20);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xF0000, (u32)(int)fmt[1] << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0xC00, (u32)(int)fmt[2] << 10);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x44, 0x300, (u32)(int)fmt[3] << 8);
}
""")
c(0x40065A60, 56, "csi_set_buffer_address", "BUFn of channel ch (+0x50 + ch*0x100 + buf*4) = 64-bit address / 4.", """
void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr)
{
    csi_clrsetbits(csi_base_tbl[sel] + (ch << 8) + (buf << 2) + 0x50, 0xFFFFFFFFu, (u32)(addr >> 2));
}
""")
c(0x40065A98, 80, "csi_capture_start", "CAP (+0x08) = mode in the byte of the channel count (1..4), 0 otherwise.", """
void csi_capture_start(u32 sel, u32 ch_total, u32 mode)
{
    u32 v;
    if (ch_total == 4) v = mode << 24;
    else if (ch_total == 3) v = mode << 16;
    else if (ch_total == 2) v = mode << 8;
    else if (ch_total == 1) v = mode;
    else v = 0;
    csi_writel_t(csi_base_tbl[sel] + 8, v);
}
""")
c(0x40065AE8, 136, "csi_set_size", "Channel size registers: HSIZE (+0x80) length, VSIZE (+0x84) length, BUF_LEN (+0x88) luma, then chroma.", """
void csi_set_size(u32 sel, u32 ch, u32 hor_len, u32 ver_len, u32 buf_len_y, u32 buf_len_c)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x80, 0x1FFF0000, hor_len << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x84, 0x1FFF0000, ver_len << 16);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x88, 0x1FFF, buf_len_y);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x88, 0x1FFF0000, buf_len_c << 16);
}
""")
c(0x40065B70, 72, "csi_set_offset", "Channel HSIZE/VSIZE start (bits 12:0).", """
void csi_set_offset(u32 sel, u32 ch, u32 hor_start, u32 ver_start)
{
    u32 o = ch << 8;
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x80, 0x1FFF, hor_start);
    csi_clrsetbits(csi_base_tbl[sel] + o + 0x84, 0x1FFF, ver_start);
}
""")
c(0x40065BB8, 28, "csi_int_enable", "Channel INT_EN (+0x70) |= bits.",
  "void csi_int_enable(u32 sel, u32 ch, u32 bits) { csi_setbits(csi_base_tbl[sel] + (ch << 8) + 0x70, bits); }")
c(0x40065BD4, 124, "csi_int_get_status", "Channel INT_STA (+0x74) bits 0..7 to eight bytes (capture done, frame done, overflows, ...).", """
void csi_int_get_status(u32 sel, u32 ch, unsigned char *st)
{
    u32 v = csi_readl_t(csi_base_tbl[sel] + (ch << 8) + 0x74);
    int i;
    for (i = 0; i < 8; i++)
        st[i] = (v >> i) & 1;
}
""")
c(0x40065C50, 20, "csi_int_clear_status", "INT_STA (+0x74) = bits; the channel argument is ignored (channel 0 register).",
  "void csi_int_clear_status(u32 sel, u32 ch, u32 bits) { (void)ch; csi_writel_t(csi_base_tbl[sel] + 0x74, bits); }")
c(0x40065C64, 4, "csi_enable_t", "b csi_enable.", """
void csi_enable(u32 sel);

void csi_enable_t(u32 sel) { csi_enable(sel); }
""")
c(0x40065C68, 24, "csi_reset", "Disable then enable.", """
void csi_enable(u32 sel);
void csi_disable(u32 sel);

void csi_reset(u32 sel)
{
    csi_disable(sel);
    csi_enable(sel);
}
""")
c(0x40065C80, 132, "csi_init_cfg", "Channel 0 format {0, 0, 2, ?}, interface {0, 4, 0}, timing {0, cfg[2], cfg[1], cfg[3]}; returns 0. The fourth format byte is uninitialised stack in the stock code (0 under the test harness; OPEN).", """
void csi_fmt_cfg(u32 sel, u32 ch, const signed char *fmt);
void csi_if_cfg(u32 sel, const signed char *cfg);
void csi_timing_cfg(u32 sel, const signed char *cfg);

int csi_init_cfg(u32 sel, const signed char *cfg)
{
    signed char fmt[4] = {0, 0, 2, 0}, ifc[4] = {0, 4, 0, 0}, tim[4];
    csi_fmt_cfg(sel, 0, fmt);
    csi_if_cfg(sel, ifc);
    tim[0] = 0; tim[1] = cfg[2]; tim[2] = cfg[1]; tim[3] = cfg[3];
    csi_timing_cfg(sel, tim);
    return 0;
}
""")
c(0x40065EA0, 4, "csi_capture_start_t", "b csi_capture_start.", """
void csi_capture_start(u32 sel, u32 ch_total, u32 mode);

void csi_capture_start_t(u32 sel, u32 ch_total, u32 mode) { csi_capture_start(sel, ch_total, mode); }
""")
c(0x40065EA4, 4, "csi_int_enable_t", "b csi_int_enable.", """
void csi_int_enable(u32 sel, u32 ch, u32 bits);

void csi_int_enable_t(u32 sel, u32 ch, u32 bits) { csi_int_enable(sel, ch, bits); }
""")
c(0x40065EA8, 4, "csi_int_get_status_t", "b csi_int_get_status.", """
void csi_int_get_status(u32 sel, u32 ch, unsigned char *st);

void csi_int_get_status_t(u32 sel, u32 ch, unsigned char *st) { csi_int_get_status(sel, ch, st); }
""")
c(0x40065EAC, 4, "csi_int_clear_status_t", "b csi_int_clear_status.", """
void csi_int_clear_status(u32 sel, u32 ch, u32 bits);

void csi_int_clear_status_t(u32 sel, u32 ch, u32 bits) { csi_int_clear_status(sel, ch, bits); }
""")
c(0x40065EB0, 72, "csi_isr", "CSI IRQ (0x74): on frame done set SYS_INFO+0x70E and acknowledge it (0x02).", """
void csi_int_get_status_t(u32 sel, u32 ch, unsigned char *st);
void csi_int_clear_status_t(u32 sel, u32 ch, u32 bits);

void csi_isr(void)
{
    unsigned char st[8];
    csi_int_get_status_t(0, 0, st);
    if (st[1] != 1) return;
    *(unsigned char *)(0x40544E68u + 0x70E) = 1;
    csi_int_clear_status_t(0, 0, 2);
}
""")
c(0x40065EF8, 80, "csi_pins_init", "PE0, PE2, PE3 function 2 (CSI PCLK/HSYNC/VSYNC), PE8..PE15 function 2 (data) via PE_CFG1 = 0x22222222.", """
void csi_pins_init(void)
{
    pio_cfg_pin((void *)0x01C20890u, 0, 2, 0);
    pio_cfg_pin((void *)0x01C20890u, 2, 2, 0);
    pio_cfg_pin((void *)0x01C20890u, 3, 2, 0);
    *(volatile u32 *)(0x01C20890u + 4) = 0x22222222u;
}
""")
c(0x40065F48, 96, "csi_clk_init", "CSI bus gate/reset, CCU 0x01C20100 bit 1, PLL at 0x01C2002C enable + wait LOCK, CSI clock 0x01C20134 = 0x81008000.", """
void ccu_gate_on(s32 code);

void csi_clk_init(void)
{
    ccu_gate_on(0x0C885888);
    csi_writel(0x01C20100u, csi_readl(0x01C20100u) | 2);
    csi_writel(0x01C2002Cu, csi_readl(0x01C2002Cu) | 0x80000000u);
    while (!(csi_readl(0x01C2002Cu) & 0x10000000u))
        ;
    csi_writel(0x01C20134u, 0x81008000u);
}
""")
c(0x40065FA8, 136, "csi_init_structs", "Fill the interface (0x42E5549C) and format (0x42E18074) descriptors, flush the D-cache, configure, then set the channel sizes (0x40065D04).", """
int csi_init_cfg(u32 sel, const signed char *cfg);
int csi_set_fmt(u32 sel, void *ifc, void *fmt);

int csi_init_structs(u32 rate)
{
    unsigned char *ifc = (unsigned char *)0x42E5549Cu, *fmt = (unsigned char *)0x42E18074u;
    *(unsigned short *)(ifc + 6) = 1;
    ifc[0] = 0; ifc[4] = 0; ifc[1] = 1; ifc[2] = 1; ifc[3] = 0;
    *(u32 *)(ifc + 0x10) = 1;
    *(u32 *)(fmt + 4) = rate;
    fmt[0] = 1; fmt[1] = 1;
    *(u32 *)(fmt + 8) = 1;
    *(u32 *)(fmt + 0x24) = 0;
    *(u32 *)(fmt + 0x28) = 0;
    *(unsigned short *)(fmt + 0x44) = 0xFFFF;
    fmt[0x4C] = 0;
    dcache_clean_inv_all();
    csi_init_cfg(0, (const signed char *)ifc);
    return csi_set_fmt(0, ifc, fmt);
}
""")
c(0x40066030, 36, "csi_set_buffer", "BUF0 of channel 0 = addr.", """
void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr);

void csi_set_buffer(u32 addr) { csi_set_buffer_address(0, 0, 0, (u64)addr); }
""")
c(0x40066054, 88, "csi_irq_setup", "Pins, ring buffer 0x42DF76D4, IRQ 0x74 -> csi_isr, IRQs on, frame-done interrupt enable.", """
void csi_pins_init(void);
void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr);
void irq_register(int irq, void (*handler)(void), int prio, int enable);
void local_irq_enable(void);
void csi_isr(void);
void csi_int_enable_t(u32 sel, u32 ch, u32 bits);

void csi_irq_setup(void)
{
    csi_pins_init();
    csi_set_buffer_address(0, 0, 0, (u64)0x42DF76D4u);
    irq_register(0x74, csi_isr, 0, 1);
    local_irq_enable();
    csi_int_enable_t(0, 0, 2);
}
""")
c(0x400660AC, 20, "enable_csi_capture", "Start capture on all configured channels in mode 1 (single).", """
void csi_capture_start_t(u32 sel, u32 ch_total, u32 mode);

void enable_csi_capture(void) { csi_capture_start_t(0, *(u32 *)(0x42E5549Cu + 0x10), 1); }
""")
c(0x400660C0, 4, "csi_pins_init_0", "mov r0, #0 then falls into 0x400660C4 (b csi_pins_init).", """
void csi_pins_init(void);

void csi_pins_init_0(void) { csi_pins_init(); }
""")
c(0x400660C4, 4, "csi_pins_init_t", "b csi_pins_init.", """
void csi_pins_init(void);

void csi_pins_init_t(void) { csi_pins_init(); }
""")
c(0x400660C8, 64, "csi_full_init", "Clock, base 0x01CB4000, reset, enable, descriptors (rate), IRQ setup. (Stock name is_a_generic_delay_helper.)", """
void csi_clk_init(void);
int csi_set_base_addr(u32 sel, u32 addr);
void csi_reset(u32 sel);
void csi_enable_t(u32 sel);
int csi_init_structs(u32 rate);
void csi_irq_setup(void);

void csi_full_init(u32 rate)
{
    csi_clk_init();
    csi_set_base_addr(0, 0x01CB4000u);
    csi_reset(0);
    csi_enable_t(0);
    csi_init_structs(rate);
    csi_irq_setup();
}
""")
c(0x40066108, 4, "csi_clear_frame_flag", "mov r0, #0 then falls into csi_set_frame_flag.", """
void csi_set_frame_flag(u32 v);

void csi_clear_frame_flag(void) { csi_set_frame_flag(0); }
""")
c(0x4006610C, 12, "csi_set_frame_flag", "SYS_INFO+0x70E = v, then flush the D-cache.", """
void csi_set_frame_flag(u32 v)
{
    *(unsigned char *)(0x40544E68u + 0x70E) = (unsigned char)v;
    dcache_clean_inv_all();
}
""")
