/*
 * bode_point_measure -- stock 0x4003A044 (652 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * One sweep point at f Hz: generator frequency word 0x42E3F984+0x2EFC = f, gen_update, 1 ms; input coupling bytes SYS+0x46 / +0x10A = 2 below 5 MHz, else 1; time-base SYS+0x588 = the first index 14..33 whose table entry (u64 SYS+0xC40 + 8 x (i - 14)) x u16 SYS+0x1E reaches 8 x 10^12 / f (else 0x21). Calibration: when the first word of record D (0x42E554B0) is below f, the entry whose frequency (words +0x10[i]) is the first >= f gives the gain ratio (+0x4C0[i]) and the phase offset (+0xE20[i]) in 0x42E180C8+0xAD08 / +0xAD10, else 1.0 / 0. Then, until both inputs are done: runtime_event_service unless byte +0xAD18 is set (touch byte +0x15 aborts: +0x47 = 0), outputs_apply, acquisition_dispatcher; for each input the range byte (SYS+0x41 for A, +0x105 for B) goes down while the amplitude word (SYS+0xB30 / +0xB4C) is above 0.8 x u16 SYS+0xA and up (to at most 11) while it is below 0.3 x; otherwise the 64-bit amplitude (SYS+0x910 / +0x970) is taken as a double into +0xACF8 / +0xAD00. At the end +0xAD00 is divided by the calibration ratio.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "bode.h"
#include "drivers/timer.h"
#include "generator.h"
#include "libc.h"
#include "system.h"
#include "sys_info.h"
#include "state/bode_block.h"
#include "state/gen_block.h"

#define SYS 0x40544E68u
#define BB 0x42E180C8u
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

void bode_point_measure(u32 f)
{
    u32 hmax, hmin, f2, n, e, a, bv, g;
    int i, done_a = 0, done_b = 0;
    unsigned long long q8;
    double h = (double)SI(plot_height);
    hmax = (u32)(h * 0.8);
    hmin = (u32)(h * 0.3);
    GENS(frequency) = f;
    gen_update();
    delay_ms(1);
    f2 = GENS(frequency);
    if ((int)f2 < 5000000) {
        SI(f_046) = 2;
        SI(f_10a) = 2;
    } else {
        SI(f_046) = 1;
        SI(f_10a) = 1;
    }
    q8 = __aeabi_uldivmod(1000000000000ULL, (unsigned long long)(long long)(int)f2) << 3;
    n = SI(f_01e);
    SI(timebase_index) = 0x21;
    for (i = 14; i < 0x22; i++)
        if (*(volatile unsigned long long *)(SYS + 0xC40 + 8 * (i - 14)) * n >= q8) {
            SI(timebase_index) = (unsigned char)i;
            break;
        }
    if (W(DREC) >= f) {
        BD(0xAD08) = 1.0;
        BD(0xAD10) = 0.0;
    } else {
        n = W(DREC + 0xC);
        for (i = 0;;) {
            if (W(DREC + 0x10 + 4 * i) >= f)
                break;
            i++;
            if ((u32)i > n)
                goto measure;
        }
        e = W(DREC + 0x10 + 4 * i);
        if (f - e < e - f)
            i--;
        BD(0xAD08) = DD(DREC + 0x4C0 + 8 * i);
        BD(0xAD10) = DD(DREC + 0xE20 + 8 * i);
    }
measure:
    for (;;) {
        if (BODES(sweep_busy) == 0) {
            runtime_event_service();
            if (B8(TOUCH + 0x15)) {
                BODES(f_047) = 0;
                break;
            }
        }
        outputs_apply();
        acquisition_dispatcher();
        a = SI(f_b30);
        bv = SI(f_b4c);
        if (!done_a) {
            g = SI(ch1_vdiv_index);
            if (a > hmax && g != 0)
                SI(ch1_vdiv_index) = (unsigned char)(g - 1);
            else if (a <= hmax && a < hmin && g <= 10)
                SI(ch1_vdiv_index) = (unsigned char)(g + 1);
            else {
                done_a = 1;
                BD(0xACF8) = (double)*(volatile long long *)(SYS + 0x910);
            }
        }
        if (!done_b) {
            g = SI(ch2_vdiv_index);
            if (bv > hmax && g != 0)
                SI(ch2_vdiv_index) = (unsigned char)(g - 1);
            else if (bv <= hmax && bv < hmin && g <= 10)
                SI(ch2_vdiv_index) = (unsigned char)(g + 1);
            else {
                done_b = 1;
                BD(0xAD00) = (double)*(volatile long long *)(SYS + 0x970);
            }
        }
        if (done_a && done_b)
            break;
    }
    BD(0xAD00) = BD(0xAD00) / BD(0xAD08);
}
