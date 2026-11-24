/*
 * bode_phase_measure -- stock 0x40039C08 (676 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Phase of sweep point idx: walks the two capture buffers (u16 samples at 0x42AC7CF4 and +0x62250, shifted right by SYS+0x1BC) from 50 x step to SYS+0x2B0 - 50 x step in steps of SYS+0x558, finds the rising crossings of the level u16 SYS+0xE on each input (at least 8 steps apart), pairs a crossing of A with the following one of B and averages their distance d; period p = mean of (last - first + 1) / (crossings - 1) of both inputs; phase = 360 d / p minus the calibration offset double 0x42E180C8+0xAD10, wrapped into -180..180, stored as s16 at +0x7D68[idx]. Divisions by 0 give 0 (ARM sdiv). The shift is assumed below 32.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define BB 0x42E180C8u
#define W(a) (*(volatile u32 *)(a))
#define DD(a) (*(volatile double *)(a))
#define BH(o) H16(BB + (o))
#define BW(o) W(BB + (o))
#define BD(o) DD(BB + (o))
#define U16(v) ((unsigned short)(v))

static int sdiv0(int a, int b)
{
    return b ? a / b : 0;
}

void bode_phase_measure(u32 idx)
{
    u32 step = SI(f_558), start = 50 * step, end, pos, sh, a, b, prev_a = 0, prev_b = 0;
    int level = SI(level_default);
    volatile unsigned short *p = (volatile unsigned short *)(0x42AC7CF4u + 2 * start);
    int na = 0, nb = 0, count = 0, sum = 0, first_a = 0, last_a = 0, first_b = 0, last_b = 0;
    int got_a = 0, got_b = 0, fl_a = 0, fl_b = 0, cnt_a = 8, cnt_b = 8, k = 0, avg, pa, pb, ph, v;
    end = SI(f_2b0) - start;
    sh = SI(f_1bc);
    if (start >= end) {
        na = 2;
        nb = 2;
    } else {
        pos = start;
        do {
            a = (unsigned short)(p[0] >> sh);
            b = (u32)H16((u32)p + 0x62250u) >> sh;
            if (k > 0) {
                if (cnt_a >= 8) {
                    if ((int)prev_a < level && (int)a >= level) {
                        if (!got_a) {
                            first_a = (int)pos;
                            got_a = 1;
                        }
                        fl_a = 1;
                        na++;
                        last_a = (int)pos;
                        cnt_a = 0;
                    }
                } else
                    cnt_a++;
                if (cnt_b >= 8) {
                    if (level > (int)(prev_b & 0xFFFF) && level <= (int)(b & 0xFFFF)) {
                        if (!got_b) {
                            first_b = (int)pos;
                            got_b = 1;
                        }
                        fl_b = 1;
                        nb++;
                        last_b = (int)pos;
                        cnt_b = 0;
                    }
                } else
                    cnt_b++;
                if (fl_a && fl_b) {
                    fl_a = 0;
                    fl_b = 0;
                    count++;
                    sum = (int)((u32)last_b + (u32)sum - (u32)last_a);
                }
            }
            prev_b = b;
            prev_a = a;
            step = SI(f_558);
            k++;
            pos += step;
            p += step;
        } while (pos < end);
        if (na <= 1)
            na = 2;
        if (nb <= 1)
            nb = 2;
    }
    avg = sdiv0(sum, count);
    pa = sdiv0(last_a - first_a + 1, na - 1);
    pb = sdiv0(last_b - first_b + 1, nb - 1);
    ph = sdiv0((int)(360u * (u32)avg), (pa + pb) / 2);
    v = (int)((double)ph - BD(0xAD10));
    if (v >= 181)
        v -= 360;
    else if (v < -180)
        v += 360;
    BH(0x7D68 + 2 * (idx & 0xFFFF)) = (unsigned short)v;
}
