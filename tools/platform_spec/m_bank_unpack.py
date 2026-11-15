# CSI bank unpacking: raw 8-bit ADC lanes of the capture banks into calibrated, interleaved
# 16-bit channel samples. exec'd by gen_platform.py


def u(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


u(0x4004073C, 444, "calibration_gain_baseline_clamp_conversion",
  "Converts count raw ADC bytes of CSI bank b (0x42DF76D4 + b x 0x2134, from byte SYS16+0x6F2 + phase) into 16-bit samples written to buf[slot], buf[slot + nb], ... (nb banks interleaved). With q = gain x cal[b] / 1000 and off = cal2[b] - q x SYS+0x1B8 (cal = 0x42E53A0C+0x10, cal2 = +0x30), each sample is base + (k x (((q x raw + off) >> SYS+0x264) - base) >> 5), clamped to [0, SYS+0x1C0 x SYS16+0xA]; gain, base and k come from the channel's V/div row (CH1: SYS+0x1C4[SYS+0x41], base SYS+0x1C0 x SYS+0x60, k SYS+0xE0; CH2: SYS+0x214[SYS+0x105], SYS+0x120, SYS+0x1A0). Stock quirk: for b > 7 or ch > 1 the code uses whatever the registers held; the C uses 0 there and the tests stay in range. (name: Hack)", """
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
""", src="acquisition; name: Hack")

CONV = "void calibration_gain_baseline_clamp_conversion(u32 b, u32 ch, volatile unsigned short *buf, u32 nb, u32 slot, u32 phase, u32 count);\n"
for (a, n, lo, hi, desc) in [(0x40040BEC, "bank_unpack_all", 0, 8, "all 8 banks (bank mode 1 or 2)"),
                             (0x40040D74, "bank_unpack_0_3", 0, 4, "banks 0..3 (CH1 in bank mode 0)"),
                             (0x40040E44, "bank_unpack_4_7", 4, 8, "banks 4..7 (CH2 in bank mode 0)")]:
    nb = hi - lo
    u(a, {8: 384, 4: 208}[nb], n,
      f"Unpacks {desc} of the channel into buf, {nb}-way interleaved: lane i uses the bank number SYS_INFO+0x6DC+i, the phase SYS+0x6D4+i and slot {hi - 1} - i; SYS16+0x6F4 samples per lane.", CONV + f"""
void {n}(volatile unsigned short *buf, u32 ch)
{{
    u32 i;
    for (i = {lo}; i < {hi}; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, {nb}, {hi - 1} - i, SYS8(0x6D4 + i), SYS16(0x6F4));
}}
""")

for (a, n, lo, hi, sz, src) in [(0x400408FC, "first_conversion_wrapper", 0, 8, 352, "acquisition; name: Hack"),
                                (0x40040A5C, "bank_unpack_0_3_n", 0, 4, 192, "acquisition"),
                                (0x40040B20, "second_conversion_wrapper", 4, 8, 192, "acquisition; name: Hack")]:
    nb = hi - lo
    u(a, sz, n,
      f"As bank_unpack for banks {lo}..{hi - 1} ({nb}-way interleave) with an explicit sample count n per lane." + (" (name: Hack)" if "Hack" in src else ""), CONV + f"""
void {n}(volatile unsigned short *buf, u32 ch, u32 n)
{{
    u32 i;
    for (i = {lo}; i < {hi}; i++)
        calibration_gain_baseline_clamp_conversion(SYS8(0x6DC + i), ch, buf, {nb}, {hi - 1} - i, SYS8(0x6D4 + i), n);
}}
""", src=src)
