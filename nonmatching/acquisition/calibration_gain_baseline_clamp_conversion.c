/*
 * calibration_gain_baseline_clamp_conversion -- stock 0x4004073C (444 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Converts count raw ADC bytes of CSI bank b (0x42DF76D4 + b x 0x2134, from byte SYS16+0x6F2 + phase) into 16-bit samples written to buf[slot], buf[slot + nb], ... (nb banks interleaved). With q = gain x cal[b] / 1000 and off = cal2[b] - q x SYS+0x1B8 (cal = 0x42E53A0C+0x10, cal2 = +0x30), each sample is base + (k x (((q x raw + off) >> SYS+0x264) - base) >> 5), clamped to [0, SYS+0x1C0 x SYS16+0xA]; gain, base and k come from the channel's V/div row (CH1: SYS+0x1C4[SYS+0x41], base SYS+0x1C0 x SYS+0x60, k SYS+0xE0; CH2: SYS+0x214[SYS+0x105], SYS+0x120, SYS+0x1A0). Stock quirk: for b > 7 or ch > 1 the code uses whatever the registers held; the C uses 0 there and the tests stay in range. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define CAL_W(o) (*(volatile int *)(0x42E53A0Cu + (o)))

void calibration_gain_baseline_clamp_conversion(u32 b, u32 ch, volatile unsigned short *buf, u32 nb,
                                                u32 slot, u32 phase, u32 count)
{
    int cg = 0, c2 = 0, gain = 0, k = 0, q, off, top, sh, v;
    u32 base = 0, stride = nb & 0xFFFF, n = count & 0xFFFF;
    const volatile unsigned char *src;
    if (b <= 7) {
        cg = CAL_W(0x10 + 4 * b);
        c2 = CAL_W(0x30 + 4 * b);
    }
    if (ch == 0) {
        gain = SYS32(0x1C4 + 4 * SYS8(0x41));
        base = SYS32(0x1C0) * SYS32(0x60);
        k = SYS32(0xE0);
    } else if (ch == 1) {
        gain = SYS32(0x214 + 4 * SYS8(0x105));
        base = SYS32(0x1C0) * SYS32(0x120);
        k = SYS32(0x1A0);
    }
    q = (int)((u32)gain * (u32)cg) / 1000;
    off = (int)((u32)c2 - (u32)q * SYS8(0x1B8));
    top = (int)(SYS32(0x1C0) * SYS16(0xA));
    sh = SYS32(0x264);
    src = (const volatile unsigned char *)(0x42DF76D4u + (int)(short)b * 0x2134 + SYS16(0x6F2) + (phase & 0xFFFF));
    buf += slot & 0xFFFF;
    for (; n != 0; n--) {
        v = (int)((u32)q * *src + (u32)off);
        v = (int)((u32)(v >> sh) - base);
        v = (int)((u32)k * (u32)v);
        v = (int)(base + (u32)(v >> 5));
        if (top < v)
            v = top;
        else if (v < 0)
            v = 0;
        *buf = v;
        buf += stride;
        src++;
    }
}
