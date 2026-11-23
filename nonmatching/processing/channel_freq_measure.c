/*
 * channel_freq_measure -- stock 0x40032B5C (480 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel ch (0, or 1 and up; 0 / 1 need SYS+0x3E / +0x102): counts the crossings of the threshold word (SYS+0xB6C / +0xB94) by the u16 samples (pointer SYS+0x7B4 + 4ch) over the middle half of SYS+0x2B0 .. SYS+0x2B4. With at least two crossings spanning at least width * SYS+0x558 / 3 samples: samples per period (float, SYS+0xB70 / +0xB98), the period 2 * span * u64 SYS+0x590 / SYS+0x1C * SYS+0x55C / SYS+0x554 / (crossings - 1) (u64, SYS+0xB80 / +0xBA8) and 10^12 / period (SYS+0xB78 / +0xBA0). Otherwise 1000.0f, 10^12 and 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

#define SB 0x40544E68u

void channel_freq_measure(u32 ch)
{
    volatile unsigned short *p = *(volatile unsigned short *volatile *)(SB + 0x7B4 + 4 * ch);
    volatile u32 *fo;
    volatile unsigned long long *o1, *o2;
    int thr, cnt = 0, first = 0, k, end;
    u32 f0 = 0, f1 = 0, e;
    if (ch == 0) {
        if (SI(ch1_enabled) == 0)
            return;
        thr = (int)SI(f_b6c);
        fo = (volatile u32 *)(SB + 0xB70);
        o2 = (volatile unsigned long long *)(SB + 0xB78);
        o1 = (volatile unsigned long long *)(SB + 0xB80);
    } else {
        if (ch == 1 && SI(ch2_enabled) == 0)
            return;
        thr = (int)SI(f_b94);
        fo = (volatile u32 *)(SB + 0xB98);
        o2 = (volatile unsigned long long *)(SB + 0xBA0);
        o1 = (volatile unsigned long long *)(SB + 0xBA8);
    }
    e = SI(f_2b4);
    k = (int)((e - SI(f_2b0)) >> 1);
    end = (int)(e - (u32)k);
    if (k < end) {
        volatile unsigned short *q = p + k;
        for (; k < end; k++, q++) {
            int a = q[0], hit;
            if (a < thr)
                hit = q[1] >= thr;
            else
                hit = thr < a && thr >= q[1];
            if (hit) {
                cnt++;
                if (first == 0) {
                    first = 1;
                    f0 = (u32)k;
                } else {
                    f1 = (u32)k;
                }
            }
        }
        if (cnt >= 2) {
            u32 diff = f1 - f0;
            if (diff >= (u32)(SI(plot_width) * SI(f_558)) / 3u) {
                int c = cnt - 1;
                unsigned long long t;
                union { float f; u32 u; } fu;
                fu.f = (float)(int)diff / (float)c;
                *fo = fu.u;
                t = (unsigned long long)(long long)(int)diff * *(volatile unsigned long long *)(SB + 0x590);
                t = t / SI(f_01c);
                t = t * SI(f_55c);
                t = t / SI(f_554);
                t = t / (unsigned long long)(long long)c;
                *o1 = t * 2u;
                *o2 = 1000000000000ull / *o1;
                return;
            }
        }
    }
    *fo = 0x447A0000u;               /* 1000.0f */
    *o1 = 1000000000000ull;
    *o2 = 0;
}
