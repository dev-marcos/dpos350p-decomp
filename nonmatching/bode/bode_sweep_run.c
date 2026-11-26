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
#include "state/bode_block.h"
#include "state/gen_block.h"
#include "shared/access.h"

#define U16(v) ((unsigned short)(v))

static const union { unsigned long long u; double d; } LIM = { 0x40B3880000000001ULL };   /* 5000.000000000001 */

static double point_ratio(void)
{
    double a = BODES(amp_a), b = BODES(amp_b), r;
    r = b / a;
    if (a >= LIM.d)
        BODES(a_reached) = 1;
    if (b >= LIM.d)
        BODES(b_reached) = 1;
    return r;
}

void bode_sweep_run(void)
{
    char buf[4];
    u32 i, n, pct;
    double l0, l1, span, step, r;
    if (g_touch->serviced)
        return;
    BODES(sweep_running) = 1;
    BODES(sweep_busy) = 0;
    BODES(freq_step) = (double)(int)(BODES(stop_freq) - BODES(start_freq)) / (double)(int)BODES(freq_count);
    GENS(amplitude) = BODES(amplitude);
    GENS(offset) = BODES(offset);
    BODES(a_reached) = 0;
    BODES(b_reached) = 0;
    l0 = log10((double)(int)BODES(start_freq));
    l1 = log10((double)(int)BODES(stop_freq));
    span = l1 - l0;
    BODES(log_end) = l1;
    BODES(log_start) = l0;
    BODES(log_span) = span;
    step = span / (double)(int)BODES(freq_count);
    font_set(0x42E658D8u);
    text_spacing_set(1);
    for (i = 0; (n = BODES(freq_count)) >= i; i++) {
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
        bode_point_measure((u32)pow(10.0, BODES(log_start) + (double)i * step));
        r = point_ratio();
        BODES(gain_ratio[i]) = r;
        BODES(gain_db[i]) = log10(r) * 20.0;
        bode_phase_measure(i & 0xFFFF);
        runtime_event_service();
        if (g_touch->busy2) {
            BODES(sweep_running) = 0;
            break;
        }
    }
    if (BODES(a_reached) == 0 || BODES(b_reached) == 0)
        for (i = 0;; ) {
            BODES(gain_ratio[i]) = 1.0;
            BODES(gain_db[i]) = log10(1.0) * 20.0;
            BODES(phase_deg[i]) = 0;
            i++;
            if (BODES(freq_count) < i)
                break;
        }
}
