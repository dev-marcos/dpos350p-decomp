/*
 * bode_sweep_run -- stock 0x4003A300 (676 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: runs a Bode sweep from word 0x42E180C8+0x54 to +0x58 Hz in n = +0x5C log steps (+0x60 = linear step, +0x8D08 / +0x8D10 / +0x8D18 = log10 of start, end and span; the generator words 0x42E3F984+0x2F00/+0x2F04 get +0x4C/+0x50). For i = 0..n: shows the progress "NN%" at (903, 510) (0..99; at 100 the previous text stays), bode_point_measure(10^(log start + i x step)), ratio B / A into +0x68[i] and 20 log10 into +0x3EE8[i], bode_phase_measure(i); touch byte +0x15 stops the sweep. Byte +0xAD19 / +0xAD1A is set when the A / B amplitude reaches 5000; unless both are set the results are reset to ratio 1, 0 dB, phase 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "bode.h"
#include "drivers/sdmmc.h"
#include "libm.h"
#include "ui/draw.h"

#define SYS 0x40544E68u
#define BB 0x42E180C8u
#define GEN 0x42E3F984u
#define DREC 0x42E554B0u
#define TOUCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W(a) (*(volatile u32 *)(a))
#define DD(a) (*(volatile double *)(a))
#define BH(o) H16(BB + (o))
#define BW(o) W(BB + (o))
#define BD(o) DD(BB + (o))
#define U16(v) ((unsigned short)(v))

static const union { unsigned long long u; double d; } LIM = { 0x40B3880000000001ULL };   /* 5000.000000000001 */

static double point_ratio(void)
{
    double a = BD(0xACF8), b = BD(0xAD00), r;
    r = b / a;
    if (a >= LIM.d)
        B8(BB + 0xAD19) = 1;
    if (b >= LIM.d)
        B8(BB + 0xAD1A) = 1;
    return r;
}

void bode_sweep_run(void)
{
    char buf[4];
    u32 i, n, pct;
    double l0, l1, span, step, r;
    if (B8(TOUCH + 0x17))
        return;
    B8(BB + 0x47) = 1;
    B8(BB + 0xAD18) = 0;
    BD(0x60) = (double)(int)(BW(0x58) - BW(0x54)) / (double)(int)BW(0x5C);
    W(GEN + 0x2F00) = BW(0x4C);
    W(GEN + 0x2F04) = BW(0x50);
    B8(BB + 0xAD19) = 0;
    B8(BB + 0xAD1A) = 0;
    l0 = log10((double)(int)BW(0x54));
    l1 = log10((double)(int)BW(0x58));
    span = l1 - l0;
    BD(0x8D10) = l1;
    BD(0x8D08) = l0;
    BD(0x8D18) = span;
    step = span / (double)(int)BW(0x5C);
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    for (i = 0; (n = BW(0x5C)) >= i; i++) {
        pct = n ? 100 * i / n : 0;
        if ((int)pct < 10) {
            buf[0] = (char)('0' + pct);
            buf[1] = '%';
            buf[2] = 0;
        } else if ((int)pct < 100) {
            buf[0] = (char)('0' + (int)pct / 10);
            buf[1] = (char)('0' + (int)pct % 10);
            buf[2] = '%';
            buf[3] = 0;
        }
        draw_target_default();
        draw_colour_set(0);
        rect_fill(0x384, 0x1FC, 0x23, 0x12);
        draw_colour_set(0xAAAAAA);
        text_draw(0x387, 0x1FE, buf);
        fb_target_back();
        dcache_clean_inv_all_t();
        bode_point_measure((u32)pow(10.0, BD(0x8D08) + (double)i * step));
        r = point_ratio();
        BD(0x68 + 8 * i) = r;
        BD(0x3EE8 + 8 * i) = log10(r) * 20.0;
        bode_phase_measure(i & 0xFFFF);
        runtime_event_service();
        if (B8(TOUCH + 0x15)) {
            B8(BB + 0x47) = 0;
            break;
        }
    }
    if (B8(BB + 0xAD19) == 0 || B8(BB + 0xAD1A) == 0)
        for (i = 0;; ) {
            BD(0x68 + 8 * i) = 1.0;
            BD(0x3EE8 + 8 * i) = log10(1.0) * 20.0;
            BH(0x7D68 + 2 * i) = 0;
            i++;
            if (BW(0x5C) < i)
                break;
        }
}
