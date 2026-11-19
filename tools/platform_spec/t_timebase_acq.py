# Time-base / acquisition helpers around SYS_INFO+0x588 (time-base index) and the "committed"
# copy at SYS_INFO+0x5A8..0x5EC, the window invalidation of ids 1..5, the trace buffer pointer
# set-up and the SPI flash page program. exec'd by gen_platform.py


def u(addr, size, name, desc, body, src="timebase"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


LOCK = "(*(volatile unsigned char *)0x42E68BC7u)"

# ---------------------------------------------------------------- small time-base helpers
u(0x4003A8BC, 36, "timebase_period_load",
  "SYS+0x590 (64 bit) = the table entry SYS+0xBD0 + 8 x SYS+0x588 (64 bit).", """
void timebase_period_load(void)
{
    SYS64(0x590) = SYS64(0xBD0 + 8u * SYS8(0x588));
}
""")
u(0x4003A908, 92, "sample_divisor_setup",
  "From the flags SYS+0x3E / SYS+0x102: mode byte SYS+0x100 (2 when 0x3E is clear, 1 when only 0x3E is set, 0 when both are set), divisor SYS+0x1B0 (2 in the last case, else 1), flag SYS+0x1AC (1 in the last case, else 0), and SYS+0x1B1 = SYS+0x524 / divisor (low byte).", """
void sample_divisor_setup(void)
{
    u32 mode, div, flag;
    if (SYS8(0x3E) == 0) {
        mode = 2;
        div = 1;
        flag = 0;
    } else if (SYS8(0x102) == 0) {
        mode = 1;
        div = 1;
        flag = 0;
    } else {
        mode = 0;
        div = 2;
        flag = 1;
    }
    SYS8(0x100) = mode;
    SYS8(0x1B0) = div;
    SYS8(0x1AC) = flag;
    SYS8(0x1B1) = SYS32(0x524) / div;
}
""", src="acquisition")
u(0x4003BE78, 32, "timebase_slow_flag_update",
  "SYS+0x28 = 1 when the time-base index SYS+0x588 >= SYS+0x2A9, else 0.", """
void timebase_slow_flag_update(void)
{
    SYS8(0x28) = SYS8(0x588) >= SYS8(0x2A9) ? 1 : 0;
}
""")
u(0x4003BEB4, 160, "runtime_state_commit_copy",
  "When SYS+0x38 is set: copies the live acquisition parameters to their committed copies (0x28->0x29, 0x550->0x5D0 (16 bit), 0x558/0x55C->0x5D4/0x5D8, 0x588->0x5B0 (byte), 0x590->0x5B8, 0x530->0x5A8, 0x570->0x5E0 (64 bit), 0x568/0x56C->0x5E8/0x5EC, 0x50->0x58, 0x60->0x64, 0x110->0x118, 0x120->0x124).", """
void runtime_state_commit_copy(void)
{
    if (SYS8(0x38) == 0)
        return;
    SYS8(0x29) = SYS8(0x28);
    SYS16(0x5D0) = SYS16(0x550);
    SYS8(0x5B0) = SYS8(0x588);
    SYS32(0x5D4) = SYS32(0x558);
    SYS32(0x5D8) = SYS32(0x55C);
    SYS64(0x5B8) = SYS64(0x590);
    SYS64(0x5A8) = SYS64(0x530);
    SYS64(0x5E0) = SYS64(0x570);
    SYS32(0x5E8) = SYS32(0x568);
    SYS32(0x5EC) = SYS32(0x56C);
    SYS64(0x58) = SYS64(0x50);
    SYS32(0x64) = SYS32(0x60);
    SYS64(0x118) = SYS64(0x110);
    SYS32(0x124) = SYS32(0x120);
}
""", src="acquisition")
u(0x4003BEA8, 12, "runtime_state_commit_force",
  "SYS+0x38 = 1, then falls into runtime_state_commit_copy (0x4003BEB4).", """
void runtime_state_commit_copy(void);

void runtime_state_commit_force(void)
{
    SYS8(0x38) = 1;
    runtime_state_commit_copy();
}
""", src="acquisition")
u(0x4003BF58, 32, "mode_not_0_or_2", "Returns 0 when v is 0 or 2, else 1.", """
u32 mode_not_0_or_2(u32 v)
{
    return (v == 0 || v == 2) ? 0 : 1;
}
""", src="acquisition")
u(0x4003BF78, 32, "mode_not_0_or_1", "Returns 0 when v is 0 or 1, else 1.", """
u32 mode_not_0_or_1(u32 v)
{
    return (v == 0 || v == 1) ? 0 : 1;
}
""", src="acquisition")
u(0x4003BF98, 112, "runtime_counter_progress_advance",
  "Sample counter step: when SYS+0x6FC >= SYS+0x708, adds SYS+0x6F8 (16 bit) to SYS+0x700 and SYS+0x56C; then SYS+0x6FC += SYS+0x6F6 (16 bit), q = SYS+0x6FC / SYS+0x1B1 (0 when the divisor is 0, as the udiv instruction gives), SYS+0x63C = q, SYS+0x704 = remainder, SYS+0x6F0 (16 bit) = SYS+0x6F4 + q.", """
void runtime_counter_progress_advance(void)
{
    u32 c = SYS32(0x6FC), d, q;
    if (c >= SYS32(0x708)) {
        u32 step = SYS16(0x6F8);
        SYS32(0x700) = SYS32(0x700) + step;
        SYS32(0x56C) = SYS32(0x56C) + step;
    }
    c += SYS16(0x6F6);
    d = SYS8(0x1B1);
    SYS32(0x6FC) = c;
    q = d ? c / d : 0;
    SYS32(0x63C) = q;
    SYS32(0x704) = c - d * q;
    SYS16(0x6F0) = SYS16(0x6F4) + q;
}
""", src="acquisition")

# ---------------------------------------------------------------- index clamp and steps
INV = """void ui_window_invalidate_from(u32 id, u32 r1);
void timebase_period_load(void);
"""
u(0x4003BCF0, 188, "timebase_index_clamp",
  "Clamps the time-base index SYS+0x588 to [SYS+0x2A8, SYS+0x2A7] (max first, then min). When SYS+0x3C (read first) is set: limits it to the committed index SYS+0x5B0 + 3, then while SYS+0x5B8 / SYS+0x590 / SYS+0x5D4 (64-bit unsigned, after timebase_period_load) >= SYS+0x2AC (16 bit) steps the index up by one and invalidates the windows from id 3 (the stock r1 is the old index).", INV + """
void timebase_index_clamp(void)
{
    u32 en = SYS8(0x3C), v, c;
    unsigned long long q;
    v = SYS8(0x2A8);
    if (v < SYS8(0x588))
        v = SYS8(0x588);
    SYS8(0x588) = v;
    v = SYS8(0x2A7);
    if (v > SYS8(0x588))
        v = SYS8(0x588);
    SYS8(0x588) = v;
    if (en == 0)
        return;
    c = SYS8(0x5B0);
    if ((int)(c + 3) < (int)v)
        SYS8(0x588) = c + 3;
    for (;;) {
        u32 lim;
        timebase_period_load();
        q = SYS64(0x5B8) / SYS64(0x590);
        lim = SYS16(0x2AC);
        q /= (u32)SYS32(0x5D4);
        if (q < lim)
            return;
        c = SYS8(0x588);
        SYS8(0x588) = c + 1;
        ui_window_invalidate_from(3, c);
    }
}
""")
u(0x4003BDB0, 48, "timebase_index_step",
  "SYS+0x588 += d (byte), timebase_index_clamp, then the 16-bit flags SYS+0x86E and SYS+0x870 = 1.", """
void timebase_index_clamp(void);

void timebase_index_step(u32 d)
{
    SYS8(0x588) = SYS8(0x588) + d;
    timebase_index_clamp();
    SYS16(0x86E) = 1;
    SYS16(0x870) = 1;
}
""")
u(0x4003BDE8, 100, "timebase_index_step_down",
  "When SYS+0x55C and SYS+0x5FC are both below SYS+0x2AC (16 bit) x SYS+0x554: SYS+0x588 -= d, or 0 when d (full 32 bits) is larger. Then timebase_index_clamp and the 16-bit flags SYS+0x86E / SYS+0x870 = 1.", """
void timebase_index_clamp(void);

void timebase_index_step_down(u32 d)
{
    u32 lim = SYS16(0x2AC) * SYS32(0x554);
    if (SYS32(0x55C) < lim && SYS32(0x5FC) < lim) {
        u32 c = SYS8(0x588);
        SYS8(0x588) = c >= d ? c - d : 0;
    }
    timebase_index_clamp();
    SYS16(0x86E) = 1;
    SYS16(0x870) = 1;
}
""")

# ---------------------------------------------------------------- acquisition sequences
u(0x4003A62C, 40, "acquisition_reconfigure",
  "sample_divisor_setup, 0x4003A680, 0x4005DB9C, sine_table_build (0x4005D27C; its first sine argument is whatever d0 holds here), acquisition_geometry_setup, then runtime_state_commit_copy.", """
void sample_divisor_setup(void);
void func_4003a680(void);
void intensity_palettes_init(void);
void sine_table_build(void);
void acquisition_geometry_setup(void);
void runtime_state_commit_copy(void);

void acquisition_reconfigure(void)
{
    sample_divisor_setup();
    func_4003a680();
    intensity_palettes_init();
    sine_table_build();
    acquisition_geometry_setup();
    runtime_state_commit_copy();
}
""", src="acquisition")
u(0x4003A654, 44, "acquisition_apply",
  "timebase_slow_flag_update, sample_divisor_setup, 0x4003A984, fpga_sample_count_write(SYS+0x30), then timebase_apply.", """
void timebase_slow_flag_update(void);
void sample_divisor_setup(void);
void func_4003a984(void);
void timebase_apply(void);

void acquisition_apply(void)
{
    timebase_slow_flag_update();
    sample_divisor_setup();
    func_4003a984();
    fpga_sample_count_write(SYS32(0x30));
    timebase_apply();
}
""", src="acquisition")
u(0x4003B7B8, 68, "acquisition_rearm",
  "Unless the byte 0x42E68BC7 is set: 0x4003B834, then 0x4004201C when SYS+0x2AA (second traces) is set.", """
void func_4003b834(void);
void func_4004201c(void);

void acquisition_rearm(void)
{
    if (""" + LOCK + """)
        return;
    func_4003b834();
    if (SYS8(0x2AA))
        func_4004201c();
}
""", src="acquisition")
u(0x4003C0AC, 172, "scaled_pair_update",
  "With n0 = SYS+0x5E8 + 1, n1 = SYS+0x5EC - 1 (signed) and k = SYS+0x5D4: when mode_not_0_or_1(mode) is 0, SYS+0x608 = n0 x k / SYS+0x5F8 and SYS+0x60C = n1 x k / SYS+0x5F8; otherwise the same with the divisor SYS+0x610 into SYS+0x620 / SYS+0x624 (64-bit signed products and divisions).", """
u32 mode_not_0_or_1(u32 v);

void scaled_pair_update(u32 mode)
{
    long long p = (long long)(int)(SYS32(0x5E8) + 1) * (long long)(u32)SYS32(0x5D4);
    long long k;
    if (mode_not_0_or_1(mode) == 0) {
        long long d = (long long)(u32)SYS32(0x5F8);
        k = (long long)(u32)SYS32(0x5D4);
        SYS32(0x608) = (u32)(p / d);
        p = (long long)(int)(SYS32(0x5EC) - 1) * k;
        SYS32(0x60C) = (u32)(p / (long long)(u32)SYS32(0x5F8));
    } else {
        long long d = (long long)(u32)SYS32(0x610);
        k = (long long)(u32)SYS32(0x5D4);
        SYS32(0x620) = (u32)(p / d);
        p = (long long)(int)(SYS32(0x5EC) - 1) * k;
        SYS32(0x624) = (u32)(p / (long long)(u32)SYS32(0x610));
    }
}
""", src="acquisition")
u(0x4003FFE4, 32, "trace_buffer_pointers_setup",
  "SYS+0x7BC = n; with a = (s16) SYS+0x29C, b = (s16) SYS+0x29E: SYS+0x7B4 = 0x42C62AF4 + 2a + 2b + 2 x ((0xC738 - n) >> 1) and SYS+0x7B8 = that + 2 x 0xC738. (The symbol table splits this function at 0x40040004; the entry here covers both pieces.)", """
void trace_buffer_pointers_setup(u32 n)
{
    u32 a = (u32)(int)(short)SYS16(0x29C), b = (u32)(int)(short)SYS16(0x29E);
    u32 p;
    SYS32(0x7BC) = n;
    p = 0x42C62AF4u + 2u * a + 2u * b + 2u * ((0xC738u - n) >> 1);
    SYS32(0x7B4) = p;
    SYS32(0x7B8) = p + 2u * 0xC738u;
}
""", src="trace")

# ---------------------------------------------------------------- UI glue
u(0x4003D454, 24, "ui_cf88_then_579c", "Calls 0x4003CF88 (role OPEN) then ui_text_init (0x4004579C).", """
void func_4003cf88(void);
void ui_text_init(void);

void ui_cf88_then_579c(void)
{
    func_4003cf88();
    ui_text_init();
}
""", src="ui")
u(0x4003D6E4, 56, "ui_windows_invalidate_1_5",
  "ui_window_invalidate_from for the ids 1..5. Stock quirk: the hidden second argument (r1) is the caller's r1 for the first call and, for the next ones, the r1 the previous call left; the C models that value (inv_r1_out).", """
#define WSTK ((volatile unsigned char *)0x42E68BC8u)
void ui_window_invalidate_from(u32 id, u32 r1);

/* r1 as ui_window_invalidate_from (0x4003CD44) leaves it */
static u32 inv_r1_out(u32 id, u32 r1)
{
    u32 i, slot = r1, n;
    for (i = 0; i < 20; i++) {
        if (WSTK[2 + i] == id) {
            slot = i;
            break;
        }
    }
    slot &= 0xFF;
    if (slot >= 20)
        return slot;
    for (n = 20 - slot; n; n--, slot++)
        if (WSTK[2 + slot] == 0)
            break;
    return n;
}

void ui_windows_invalidate_1_5(u32 r0, u32 r1)
{
    u32 id;
    (void)r0;
    for (id = 1; id <= 5; id++) {
        ui_window_invalidate_from(id, r1);
        r1 = inv_r1_out(id, r1);
    }
}
""", src="ui")

# ---------------------------------------------------------------- SPI flash page program
u(0x4004372C, 200, "spi_flash_page_program",
  "Programs len bytes from buf at addr: write enable, wait, select, command 0x02 + 3 address bytes (or 0x12 + 4 when the word 0x42E68CD4 is set) followed by the data in one sys_spi_write_then_write, deselect, wait. Returns 0, or -1 when the write enable or the transfer failed.", """
void sys_spi_select(u32 dev);
void sys_spi_deselect(u32 dev);
int spi_flash_write_enable(u32 dev);
void spi_flash_wait_ready(u32 dev);
int sys_spi_write_then_write(int spi, const unsigned char *tx0, int len0, const unsigned char *tx1, int len1);

int spi_flash_page_program(u32 dev, u32 addr, const unsigned char *buf, int len)
{
    volatile unsigned char cmd[8];
    int ret = 0, n;
    if (*(volatile u32 *)0x42E68CD4u == 0) {
        cmd[0] = 0x02;
        cmd[1] = addr >> 16;
        cmd[2] = addr >> 8;
        cmd[3] = addr;
        n = 4;
    } else {
        cmd[0] = 0x12;
        cmd[1] = addr >> 24;
        cmd[2] = addr >> 16;
        cmd[3] = addr >> 8;
        cmd[4] = addr;
        n = 5;
    }
    if (spi_flash_write_enable(dev) < 0)
        ret = -1;
    spi_flash_wait_ready(dev);
    sys_spi_select(dev);
    if (sys_spi_write_then_write((int)dev, (const unsigned char *)cmd, n, buf, len) < 0)
        ret = -1;
    sys_spi_deselect(dev);
    spi_flash_wait_ready(dev);
    return ret;
}
""", src="spi")
