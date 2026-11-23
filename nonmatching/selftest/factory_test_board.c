/*
 * factory_test_board -- stock 0x40050FE0 (1416 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * First factory step (board test). On a cleared back buffer, in white with font 0x42E658D8 and spacing 1, each line prints a label at x = 15 (y = 15, 40, .. step 25), runs a test and prints 成功 (white) or a red failure at x = 160; any failure parks in for (;;): disk_check_file (flag 0x42E68AF0), fpga_probe_and_copy_7fc (+0x11), security_check_and_copy (+0x12), table_684_init + selftest_4f5e4 (+0x18; on failure ADC1..ADC8 for each clear flag +0x19..+0x20, one line each), calibration_result_flags + acq_defaults_reset + calib_run_4_channels with SYS+0x54E = 1 (+0x21, else channel 1 / 2 from +0x22 / +0x23), gen_selftest_4f89c (+0x13), gen_selftest_50bac (+0x3F, else +0x40 / +0x41). On success SI(f_780) = SI(f_77c).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "selftest.h"
#include "storage/diskio.h"
#include "system.h"
#include "ui/draw.h"
#include "sys_info.h"
#include "state/factory_block.h"

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
    if (!FACT(f_010)) {
        ft_text(FT_RED, 160, 15, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 15, (const char *)0x4008EFDCu);     /* FPGA检测 ... */
    fpga_probe_and_copy_7fc();
    if (!FACT(fpga_probe_ok)) {
        ft_text(FT_RED, 160, 40, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 40, (const char *)0x4008E304u);     /* 加密芯片检测 ... */
    security_check_and_copy();
    if (!FACT(security_chip_ok)) {
        ft_text(FT_RED, 160, 65, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 65, (const char *)0x4008EFECu);     /* ADC检测 ... */
    table_684_init();
    selftest_4f5e4();
    if (!FACT(f_018)) {
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
    SI(f_54e) = 1;
    calib_run_4_channels();
    SI(f_54e) = 0;
    if (!FACT(f_021))
        ft_fail2(160, 115, FACT(f_022), FACT(f_023));
    ft_ok_next(160, 115, (const char *)0x4008E31Cu);    /* 通道复用检测 ... */
    gen_selftest_4f89c();
    if (!FACT(f_013)) {
        ft_text(FT_RED, 160, 140, FT_FAIL);
        ft_halt();
    }
    ft_ok_next(160, 140, (const char *)0x4008DC40u);    /* 150M带宽限制检测 ... */
    gen_selftest_50bac();
    if (!FACT(f_03f))
        ft_fail2(160, 165, FACT(f_040), FACT(f_041));
    ft_text(FT_WHITE, 160, 165, FT_OK);
    SI(f_780) = SI(f_77c);
}
