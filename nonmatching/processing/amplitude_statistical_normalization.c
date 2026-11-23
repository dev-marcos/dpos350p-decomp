/*
 * amplitude_statistical_normalization -- stock 0x4003FD10 (260 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Measurement class for a channel (SYS_INFO+0x51C / +0x51D): 0 when the sample span is below 22 MHz / rate x SYS+0x1B1 or the amplitude / SYS+0x1C0 is below 25, else 2 when the span reaches 250 MHz / rate x SYS+0x1B1 and SYS+0x55C x scale / SYS+0x554 >= 23, else 1. (name: Hack FPGA.md; the code classifies rather than normalises)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void amplitude_statistical_normalization(u32 ch)
{
    volatile unsigned char *dst;
    u32 amp, lo, hi, rate, r5, res;
    float s0, s2;
    unsigned long long span, p;
    if (ch == 0) {
        dst = (volatile unsigned char *)(0x40544E68u + 0x51C);
        amp = SI(f_b68);
        lo = SI(f_b78); hi = SI(f_b7c);
        s0 = *(volatile float *)(0x40544E68u + 0xB70);
    } else {
        dst = (volatile unsigned char *)(0x40544E68u + 0x51D);
        amp = SI(f_b90);
        s0 = *(volatile float *)(0x40544E68u + 0xB98);
        lo = SI(f_ba0); hi = SI(f_ba4);
    }
    s0 = (float)SI(f_55c) * s0;
    rate = SI(f_524);
    s2 = (float)SI(f_554);
    p = (unsigned long long)(22000000u / rate) * SI(repeat_factor);
    s2 = s0 / s2;
    r5 = (u32)(int)s2;
    span = ((unsigned long long)hi << 32) | lo;
    if (span < p)
        res = 0;
    else {
        p = (unsigned long long)(250000000u / rate) * SI(repeat_factor);
        if (span >= p && (int)r5 >= 23)
            res = 2;
        else
            res = 1;
    }
    *dst = res;
    if ((int)(amp / SI(acq_geometry_1c0)) < 25)
        *dst = 0;
}
