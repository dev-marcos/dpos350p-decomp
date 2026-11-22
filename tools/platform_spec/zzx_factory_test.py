# Factory test flow run by factory_steps_run (0x40051CF0) until each step's done-word is set:
# the board test (disk, FPGA, crypto chip, ADC lanes, system calibration, channel mux, 150 MHz
# bandwidth limit), then the tester test (wait for OK on the touch screen, channel and generator
# loop-back, ranges, DC/AC coupling, flatness, 20 MHz limit, ADC clock phase).
# Each step prints a Chinese label at x = 15, runs one self-test and prints 成功 (OK, white) at
# x = 160 / 220 or a red failure text, then parks in `for (;;)` on any failure.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wx(addr, size, name, desc, body, src="init"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


FT = r"""
void fb_target_back(void);
void fb_clear_current(void);
void fb_present(void);
void draw_colour_set(u32 c);
void word_42e68c74_set(u32 a);
void half_42e68be2_set(u32 a);
void text_draw(u32 x, u32 y, const char *s);

#define FT8(o)      (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define FT_WHITE    0x00FFFFFFu
#define FT_RED      0x00FF2222u
#define FT_FONT     0x42E658D8u
#define FT_OK       ((const char *)0x4008FFC8u)     /* 成功 */
#define FT_FAIL     ((const char *)0x4008FFD0u)     /* 异常 */
#define FT_CH1_FAIL ((const char *)0x4008EFFCu)     /* 通道1异常 */
#define FT_CH2_FAIL ((const char *)0x4008F00Cu)     /* 通道2异常 */

static void ft_text(u32 colour, u32 x, u32 y, const char *s)
{
    draw_colour_set(colour);
    text_draw(x, y, s);
    fb_present();
}

/* white "OK" at (x, y), then the next step's label at (15, y + 25) */
static void ft_ok_next(u32 x, u32 y, const char *label)
{
    ft_text(FT_WHITE, x, y, FT_OK);
    text_draw(15, y + 25, label);
    fb_present();
}

static void ft_halt(void)
{
    for (;;)
        ;
}

/* red "channel 1 failed" / "channel 2 failed" (the second one 25 lower when both are shown) */
static void ft_fail2(u32 x, u32 y, u32 ok1, u32 ok2)
{
    if (!ok1) {
        ft_text(FT_RED, x, y, FT_CH1_FAIL);
        y += 25;
    }
    if (!ok2)
        ft_text(FT_RED, x, y, FT_CH2_FAIL);
    ft_halt();
}
"""

wx(0x40050FE0, 1416, "factory_test_board",
   "First factory step (board test). On a cleared back buffer, in white with font 0x42E658D8 and spacing 1, each line "
   "prints a label at x = 15 (y = 15, 40, .. step 25), runs a test and prints 成功 (white) or a red failure at x = 160; "
   "any failure parks in for (;;): disk_check_file (flag 0x42E68AF0), fpga_probe_and_copy_7fc (+0x11), "
   "security_check_and_copy (+0x12), table_684_init + selftest_4f5e4 (+0x18; on failure ADC1..ADC8 for each clear "
   "flag +0x19..+0x20, one line each), calibration_result_flags + acq_defaults_reset + calib_run_4_channels with "
   "SYS+0x54E = 1 (+0x21, else channel 1 / 2 from +0x22 / +0x23), gen_selftest_4f89c (+0x13), gen_selftest_50bac "
   "(+0x3F, else +0x40 / +0x41). On success SYS32(0x780) = SYS32(0x77C).", FT + r"""
void disk_check_file(void);
void fpga_probe_and_copy_7fc(void);
void security_check_and_copy(void);
void table_684_init(void);
void selftest_4f5e4(void);
void calibration_result_flags(void);
void acq_defaults_reset(void);
void calib_run_4_channels(void);
void gen_selftest_4f89c(void);
void gen_selftest_50bac(void);

#define TEXT_W (*(volatile unsigned short *)0x42E68BE4u)

void factory_test_board(void)
{
    u32 y, k;
    fb_target_back();
    fb_clear_current();
    draw_colour_set(FT_WHITE);
    word_42e68c74_set(FT_FONT);
    half_42e68be2_set(1);
    text_draw(15, 15, (const char *)0x4008E8E8u);       /* 磁盘初始化 ... */
    fb_present();
    disk_check_file();
    if (!FT8(0x10)) {
        ft_text(FT_RED, 160, 15, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 15, (const char *)0x4008EFDCu);     /* FPGA检测 ... */
    fpga_probe_and_copy_7fc();
    if (!FT8(0x11)) {
        ft_text(FT_RED, 160, 40, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 40, (const char *)0x4008E304u);     /* 加密芯片检测 ... */
    security_check_and_copy();
    if (!FT8(0x12)) {
        ft_text(FT_RED, 160, 65, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 65, (const char *)0x4008EFECu);     /* ADC检测 ... */
    table_684_init();
    selftest_4f5e4();
    if (!FT8(0x18)) {
        /* one red line "ADCk 异常" per failed lane; names "ADC1".."ADC8" are 8 bytes apart */
        y = 90;
        for (k = 0; k < 8; k++) {
            if (FT8(0x19 + k))
                continue;
            draw_colour_set(FT_RED);
            text_draw(160, y, (const char *)(0x4008FFD8u + 8 * k));
            text_draw((TEXT_W + (k ? 0xA1 : 0xA6)) & 0xFFFF, y, FT_FAIL);
            fb_present();
            y += 25;
        }
        ft_halt();
    }
    ft_ok_next(160, 90, (const char *)0x4008E8FCu);     /* 系统校准 ... */
    calibration_result_flags();
    acq_defaults_reset();
    SYS8(0x54E) = 1;
    calib_run_4_channels();
    SYS8(0x54E) = 0;
    if (!FT8(0x21))
        ft_fail2(160, 115, FT8(0x22), FT8(0x23));
    ft_ok_next(160, 115, (const char *)0x4008E31Cu);    /* 通道复用检测 ... */
    gen_selftest_4f89c();
    if (!FT8(0x13)) {
        ft_text(FT_RED, 160, 140, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 140, (const char *)0x4008DC40u);    /* 150M带宽限制检测 ... */
    gen_selftest_50bac();
    if (!FT8(0x3F))
        ft_fail2(160, 165, FT8(0x40), FT8(0x41));
    ft_text(FT_WHITE, 160, 165, FT_OK);
    SYS32(0x780) = SYS32(0x77C);
}
""")

wx(0x40051578, 1128, "factory_test_tester",
   "Second factory step (with the external tester). Prints 请插入测试器并按 OK 键继续... (insert the tester and press "
   "OK), a grey (0x555555) 120 x 120 button at (452, 240) labelled OK (font 0x42E65D98, spacing 3, at (502, 290)), "
   "then polls gt911_poll until a release (state 2) inside it (453 <= x < 572, 241 <= y < 360). Then it resets the "
   "settings (acq_defaults_reset, the three state blocks, outputs_apply), runs gen_output_calibrate, bode_calibrate "
   "and factory_channel_gen_check (+0x24, else +0x25 / +0x26: channel 1, channel 2 or 通道或信号发生器异常), "
   "gen_selftest_50688 (ranges, +0x2E / +0x2F / +0x30), gen_selftest_504b8 (DC/AC, +0x27 / +0x28 / +0x29), "
   "gen_selftest_50898 (flatness, +0x34 / +0x35 / +0x36), gen_selftest_50ddc (20 MHz limit, +0x42 / +0x43 / +0x44) "
   "and check_pair_save (ADC clock phase, +0x45); results at x = 220, failures park in for (;;).", FT + r"""
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void gt911_poll(void);
void acq_defaults_reset(void);
void state_block_3f984_init(void);
void state_block_180c8_init(void);
void state_block_68668_init(void);
void outputs_apply(void);
void gen_output_calibrate(void);
void bode_calibrate(void);
void factory_channel_gen_check(void);
void gen_selftest_50688(void);
void gen_selftest_504b8(void);
void gen_selftest_50898(void);
void gen_selftest_50ddc(void);
void check_pair_save(void);

#define TS8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))
#define TS16(o) (*(volatile short *)(0x42E68BB0u + (o)))

void factory_test_tester(void)
{
    fb_target_back();
    fb_clear_current();
    draw_colour_set(FT_WHITE);
    word_42e68c74_set(FT_FONT);
    half_42e68be2_set(1);
    text_draw(15, 15, (const char *)0x4008D32Cu);       /* 请插入测试器并按 OK 键继续... */
    draw_colour_set(0x555555);
    rect_fill(452, 240, 120, 120);
    draw_colour_set(FT_WHITE);
    half_42e68be2_set(3);
    word_42e68c74_set(0x42E65D98u);
    text_draw(502, 290, (const char *)0x40051964u);     /* "OK" */
    half_42e68be2_set(1);
    word_42e68c74_set(FT_FONT);
    fb_present();
    do
        gt911_poll();
    while (TS8(0xC) != 2 || (u32)(TS16(4) - 453) >= 0x77 || (u32)(TS16(6) - 241) >= 0x77);

    fb_clear_current();
    text_draw(15, 15, (const char *)0x4008D7B0u);       /* 通道和信号发生器检测 ... */
    fb_present();
    acq_defaults_reset();
    state_block_3f984_init();
    state_block_180c8_init();
    state_block_68668_init();
    outputs_apply();
    gen_output_calibrate();
    bode_calibrate();
    factory_channel_gen_check();
    if (!FT8(0x24)) {
        if (!FT8(0x25))
            ft_text(FT_RED, 220, 15, FT8(0x26) ? FT_CH1_FAIL : (const char *)0x4008DA00u);  /* 通道或信号发生器异常 */
        else if (!FT8(0x26))
            ft_text(FT_RED, 220, 15, FT_CH2_FAIL);
        else
            fb_present();
        ft_halt();
    }
    ft_ok_next(220, 15, (const char *)0x4008E334u);     /* 通道量程检测 ... */
    gen_selftest_50688();
    if (!FT8(0x2E))
        ft_fail2(220, 40, FT8(0x2F), FT8(0x30));
    ft_ok_next(220, 40, (const char *)0x4008E34Cu);     /* 通道DCAC检测 ... */
    gen_selftest_504b8();
    if (!FT8(0x27))
        ft_fail2(220, 65, FT8(0x28), FT8(0x29));
    ft_ok_next(220, 65, (const char *)0x4008DC5Cu);     /* 通道平坦度检测 ... */
    gen_selftest_50898();
    if (!FT8(0x34))
        ft_fail2(220, 90, FT8(0x35), FT8(0x36));
    ft_ok_next(220, 90, (const char *)0x4008DC78u);     /* 20M带宽限制检测 ... */
    gen_selftest_50ddc();
    if (!FT8(0x42))
        ft_fail2(220, 115, FT8(0x43), FT8(0x44));
    ft_ok_next(220, 115, (const char *)0x4008DC94u);    /* ADC时钟相位校准 ... */
    check_pair_save();
    if (!FT8(0x45)) {
        ft_text(FT_RED, 220, 140, FT_FAIL);
        ft_halt();
    }
    ft_text(FT_WHITE, 220, 140, FT_OK);
}
""")

wx(0x4004FB44, 2396, "factory_channel_gen_check",
   "Generator-to-channel loop-back check of the tester step. For 9 settings (sine, 0x42E4287E = 0; frequency Hz "
   "0x42E42880, amplitude mVpp +4, offset mV +8; time base SYS+0x588; volts/div SYS+0x41 = SYS+0x105) it runs "
   "outputs_apply, gen_update and acquisition_dispatcher twice and checks the measurements / 1000 (CH1: frequency "
   "s64 SYS+0x918, amplitude +0x910, mean +0x8F8; CH2: +0x978, +0x970, +0x958) against the set value +- t % "
   "(for a negative offset the window is mirrored). Settings 1..4 (10 kHz, 1250 / 500 mVpp, +-1000 / +-400 mV) check "
   "all three, 5..7 (50 MHz 2500 / 1000 mVpp at 70 %, 10 Hz 1000 mVpp) frequency and amplitude, 8..9 (+-2500 mV DC) "
   "the mean. Flags 0x42E68AE0+0x25 / +0x26 = all CH1 / CH2 checks passed, +0x24 = both. It first runs "
   "acq_defaults_reset and sets SYS+0x3E = SYS+0x102 = 1, SYS+0x660 = 5, byte 0x42E68BC6 = 0.", r"""
void acq_defaults_reset(void);
void outputs_apply(void);
void gen_update(void);
void acquisition_dispatcher(void);
long long __aeabi_ldivmod(long long n, long long d);

#define GEN8(o)  (*(volatile unsigned char *)(0x42E42880u + (o)))
#define GEN32(o) (*(volatile int *)(0x42E42880u + (o)))
#define CG_MEAS(o) ((int)__aeabi_ldivmod((long long)SYS64(o), 1000))

enum { CG_FREQ = 1, CG_AMP = 2, CG_MEAN = 4 };

/* set value v +- t % (t mirrored for a negative v) contains m */
static int cg_in(int m, int v, int t)
{
    int lo, hi;
    if (v < 0)
        t = -t;
    lo = (int)__aeabi_ldivmod((long long)v * (100 - t), 100);
    hi = (int)__aeabi_ldivmod((long long)v * (100 + t), 100);
    return m >= lo && hi >= m;
}

/* one setting; returns bit 0 = CH1 passed, bit 1 = CH2 passed */
static u32 cg_point(int freq, int amp, int offs, u32 tb, u32 vdiv, int t, u32 what)
{
    int ok1 = 1, ok2 = 1;
    GEN8(-2) = 0;
    GEN32(0) = freq;
    GEN32(4) = amp;
    GEN32(8) = offs;
    SYS8(0x588) = tb;
    SYS8(0x41) = vdiv;
    SYS8(0x105) = vdiv;
    outputs_apply();
    gen_update();
    acquisition_dispatcher();
    acquisition_dispatcher();
    if (what & CG_FREQ) {
        ok1 &= cg_in(CG_MEAS(0x918), freq, t);
        ok2 &= cg_in(CG_MEAS(0x978), freq, t);
    }
    if (what & CG_AMP) {
        ok1 &= cg_in(CG_MEAS(0x910), amp, t);
        ok2 &= cg_in(CG_MEAS(0x970), amp, t);
    }
    if (what & CG_MEAN) {
        ok1 &= cg_in(CG_MEAS(0x8F8), offs, t);
        ok2 &= cg_in(CG_MEAS(0x958), offs, t);
    }
    return ok1 | ok2 << 1;
}

void factory_channel_gen_check(void)
{
    u32 r = 3;
    acq_defaults_reset();
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)0x42E68BC6u = 0;
    r &= cg_point(10000, 1250, 1000, 24, 5, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(10000, 1250, -1000, 24, 5, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(10000, 500, 400, 24, 6, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(10000, 500, -400, 24, 6, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(50000000, 2500, 0, 13, 5, 70, CG_FREQ | CG_AMP);
    r &= cg_point(50000000, 1000, 0, 13, 6, 70, CG_FREQ | CG_AMP);
    r &= cg_point(10, 1000, 0, 32, 6, 15, CG_FREQ | CG_AMP);
    r &= cg_point(0, 0, 2500, 24, 4, 15, CG_MEAN);
    r &= cg_point(0, 0, -2500, 24, 4, 15, CG_MEAN);
    FT8(0x25) = r & 1;
    FT8(0x26) = r >> 1;
    FT8(0x24) = r == 3;
}
""".replace("#define GEN8", "#define FT8(o)   (*(volatile unsigned char *)(0x42E68AE0u + (o)))\n#define GEN8"))
