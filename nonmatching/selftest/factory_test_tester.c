/*
 * factory_test_tester -- stock 0x40051578 (1128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Second factory step (with the external tester). Prints 请插入测试器并按 OK 键继续... (insert the tester and press OK), a grey (0x555555) 120 x 120 button at (452, 240) labelled OK (font 0x42E65D98, spacing 3, at (502, 290)), then polls gt911_poll until a release (state 2) inside it (453 <= x < 572, 241 <= y < 360). Then it resets the settings (acq_defaults_reset, the three state blocks, outputs_apply), runs gen_output_calibrate, bode_calibrate and factory_channel_gen_check (+0x24, else +0x25 / +0x26: channel 1, channel 2 or 通道或信号发生器异常), gen_selftest_50688 (ranges, +0x2E / +0x2F / +0x30), gen_selftest_504b8 (DC/AC, +0x27 / +0x28 / +0x29), gen_selftest_50898 (flatness, +0x34 / +0x35 / +0x36), gen_selftest_50ddc (20 MHz limit, +0x42 / +0x43 / +0x44) and check_pair_save (ADC clock phase, +0x45); results at x = 220, failures park in for (;;).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "bode.h"
#include "drivers/gt911.h"
#include "generator.h"
#include "selftest.h"
#include "storage/files.h"
#include "system.h"
#include "ui/draw.h"
#include "state/factory_block.h"

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
    if (!FACT(f_024)) {
        if (!FACT(ch1_checks_ok))
            ft_text(FT_RED, 220, 15, FACT(ch2_checks_ok) ? FT_CH1_FAIL : (const char *)0x4008DA00u);  /* 通道或信号发生器异常 */
        else if (!FACT(ch2_checks_ok))
            ft_text(FT_RED, 220, 15, FT_CH2_FAIL);
        else
            fb_present();
        ft_halt();
    }
    ft_ok_next(220, 15, (const char *)0x4008E334u);     /* 通道量程检测 ... */
    gen_selftest_50688();
    if (!FACT(f_02e))
        ft_fail2(220, 40, FACT(f_02f), FACT(f_030));
    ft_ok_next(220, 40, (const char *)0x4008E34Cu);     /* 通道DCAC检测 ... */
    gen_selftest_504b8();
    if (!FACT(f_027))
        ft_fail2(220, 65, FACT(f_028), FACT(f_029));
    ft_ok_next(220, 65, (const char *)0x4008DC5Cu);     /* 通道平坦度检测 ... */
    gen_selftest_50898();
    if (!FACT(f_034))
        ft_fail2(220, 90, FACT(window_a_ok), FACT(window_b_ok));
    ft_ok_next(220, 90, (const char *)0x4008DC78u);     /* 20M带宽限制检测 ... */
    gen_selftest_50ddc();
    if (!FACT(f_042))
        ft_fail2(220, 115, FACT(f_043), FACT(f_044));
    ft_ok_next(220, 115, (const char *)0x4008DC94u);    /* ADC时钟相位校准 ... */
    check_pair_save();
    if (!FACT(f_045)) {
        ft_text(FT_RED, 220, 140, FT_FAIL);
        ft_halt();
    }
    ft_text(FT_WHITE, 220, 140, FT_OK);
}
