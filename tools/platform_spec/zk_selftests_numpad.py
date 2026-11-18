# Two more self-tests (output levels, capture bit lines), the numpad text parser and the limits of
# the measurement-window values entered on the numpad.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wk(addr, size, name, desc, body, src="init"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wk(0x4004F89C, 640, "gen_selftest_4f89c",
   "Self-test (DERIVED): range SYS+0x588 = 0x18, levels SYS+0x41 = SYS+0x105 = 6, SYS+0x660 = 5, byte 0x42E68BC6 = 0. Four output settings, each around the centre e = SYS+0xE with the step h = SYS+0x20 (offsets SYS+0x60 / SYS+0x120 = e +/- 2h in turn, channels SYS+0x3E / SYS+0x102 on and off), each applied and followed by two acquisitions. Checks of the measured words SYS+0xB34 and SYS+0xB50 against e * 1.3 and e * 0.7 (doubles); byte 0x42E68AF3 = 1 when all pass.",
   """
void acq_defaults_reset(void);
void outputs_apply(void);
void acquisition_dispatcher(void);

static void apply2(void)
{
    outputs_apply();
    acquisition_dispatcher();
    acquisition_dispatcher();
}

void gen_selftest_4f89c(void)
{
    u32 e, h, v, p1, p2, p3, p4;
    acq_defaults_reset();
    e = SYS16(0xE);
    SYS8(0x588) = 0x18;
    SYS8(0x41) = 6;
    SYS8(0x105) = 6;
    h = SYS16(0x20);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS32(0x120) = e - 2 * h;
    SYS32(0x60) = e + 2 * h;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    apply2();

    e = SYS16(0xE);
    p1 = (double)e * 1.3 < (double)SYS32(0xB34) && (double)SYS32(0xB50) < (double)e * 0.7;
    h = SYS16(0x20);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS32(0x60) = e - 2 * h;
    SYS32(0x120) = e + 2 * h;
    apply2();

    e = SYS16(0xE);
    p2 = (double)e * 1.3 < (double)SYS32(0xB50) && (double)SYS32(0xB34) < (double)e * 0.7;
    h = SYS16(0x20);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS32(0x60) = e + 2 * h;
    SYS32(0x120) = e - 2 * h;
    apply2();

    e = SYS16(0xE);
    v = SYS32(0xB34);
    h = SYS16(0x20);
    SYS8(0x3E) = 0;
    SYS8(0x102) = 1;
    SYS32(0x60) = e + 2 * h;
    SYS32(0x120) = e - 2 * h;
    p3 = (double)e * 1.3 < (double)v;
    apply2();

    e = SYS16(0xE);
    p4 = (double)SYS32(0xB50) < (double)e * 0.7;
    *(volatile unsigned char *)(0x42E68AE0u + 0x13) = p4 & (p3 & (p2 & p1));
}
""")

wk(0x4004F5E4, 668, "selftest_4f5e4",
   "Self-test of the eight capture byte lanes (DERIVED): channel 1 only (SYS+0x3E = 1, SYS+0x102 = 0, SYS+0x3A = 1), range 0x18, level 6; s64 SYS+0x30 = 10, SYS+0x640 = 10, SYS+0x644 = 1, stream geometry sent. Then 900 steps of the FPGA 0x2A value SYS+0x44 = (int)(3000.0f + i * 7.7777777f): each sends it, prepares and runs one capture and copies byte 0x42DF76D4 + k * 0x2134 of the eight lanes k to 0x42E51EB4 + k * 1000 + i. A lane passes (byte 0x42E68AF9 + k = 1) when, over its bytes 10..889, every bit is seen both set and clear and every pair of neighbouring bits is seen different; byte 0x42E68AF8 = 1 when all eight pass.",
   """
void acq_defaults_reset(void);
void outputs_apply(void);
void stream_geometry_fpga_selector_0x22_configuration(void);
void fpga_write_sel_0x2a(void);
void delay_ms(u32 ms);
void acquisition_prepare(void);
void trigger_pos_update(void);
void capture_banks_all(void);
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define LANE8(o) (*(volatile unsigned char *)(0x42E51EB4u + (o)))

void selftest_4f5e4(void)
{
    unsigned char set[8], clr[8], edge[8];
    u32 k, i, b, ok;
    int s;
    for (k = 0; k < 8; k++) {
        set[k] = clr[k] = edge[k] = 0;
        RS8(0x19 + k) = 0;
    }
    acq_defaults_reset();
    SYS8(0x588) = 0x18;
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS8(0x41) = 6;
    SYS8(0x3A) = 1;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    outputs_apply();
    *(volatile long long *)(0x40544E68u + 0x30) = 10;
    SYS16(0x640) = 10;
    SYS16(0x644) = 1;
    stream_geometry_fpga_selector_0x22_configuration();
    SYS16(0x44) = 0xBB8;
    fpga_write_sel_0x2a();
    delay_ms(200);
    for (s = 0; s < 900; s++) {
        SYS16(0x44) = (int)(3000.0f + (float)s * 7.7777777f);
        fpga_write_sel_0x2a();
        acquisition_prepare();
        SYS32(0x63C) = 0;
        trigger_pos_update();
        capture_banks_all();
        for (k = 0; k < 8; k++)
            LANE8(k * 1000 + s) = *(volatile unsigned char *)(0x42DF76D4u + k * 0x2134);
    }
    for (k = 0; k < 8; k++)
        for (b = 0; b < 8; b++)
            for (i = 0; i < 880; i++) {
                if (LANE8(k * 1000 + 10 + i) & (1u << b))
                    set[k] |= 1u << b;
                else
                    clr[k] |= 1u << b;
            }
    for (k = 0; k < 8; k++)
        for (i = 0; i < 880; i++) {
            u32 x = LANE8(k * 1000 + 10 + i);
            for (b = 0; b <= 6; b++) {
                u32 m = (1u << b) & 0xFF, n = (1u << (b + 1)) & 0xFF;
                if (((x & m) != 0) != ((x & n) != 0)) {
                    u32 e = edge[k] | m;
                    if (b == 6)
                        e |= n;
                    edge[k] = e;
                }
            }
        }
    for (k = 0; k < 8; k++)
        if ((unsigned char)(edge[k] & set[k] & clr[k]) == 0xFF)
            RS8(0x19 + k) = 1;
    ok = 1;
    for (k = 0; k < 8; k++) {
        RS8(0x18) = ok;
        ok &= RS8(0x19 + k);
    }
    RS8(0x18) = ok;
}
""")

wk(0x4005FC84, 520, "numpad_value_parse",
   "Numpad text to value: the text at 0x42E68A8B (an optional '-', then up to 20 characters; digits until the first other character, a '.' switches to decimals) becomes a double; by the numpad mode byte 0x42E68A89 and the unit key (0xE, 0xF, 0x10 of numpad_key) it is scaled by 1, 1000, 1e6 or 10, or the old value 0x42E68AA8 is kept. When the text is not empty the double goes to the target *0x42E68AB0, the value clamped to -0x7FFFFFFF..0x7FFFFFFF as s64 to *0x42E68AB4 and its low word to *0x42E68AB8, and the callback 0x42E68AD8 runs if flag 0x42E68AD4 is set.",
   """
#define NP8(o) (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define NPS8(o) (*(volatile signed char *)(0x42E68A88u + (o)))
#define NP32(o) (*(volatile u32 *)(0x42E68A88u + (o)))
#define NPD(o) (*(volatile double *)(0x42E68A88u + (o)))

void numpad_value_parse(u32 unit)
{
    double v = NPD(0x20), acc = 0.0, x;
    unsigned long long scale = 1, f = 1;
    int neg = NPS8(3) == '-', i;
    u32 m;
    for (i = neg; i < 20; i++) {
        u32 ch = (unsigned short)NPS8(3 + i);
        if (ch == '.') {
            f = 10;
            continue;
        }
        if (ch - 0x30 >= 10)
            break;
        scale *= f;
        if (neg)
            acc = acc * 10.0 - (double)(unsigned short)(ch - 0x30);
        else
            acc = acc * 10.0 + (double)(unsigned short)(ch - 0x30);
    }
    x = acc / (double)scale;
    m = NP8(1);
    if (unit == 0xE && m >= 1 && m <= 5)
        v = x;
    else if (m == 1 && unit == 0xF)
        v = x * 1000.0;
    else if (m == 1 && unit == 0x10)
        v = x * 1000000.0;
    else if (m == 4 && unit == 0xF)
        v = x * 1000.0;
    else if (m == 6 && unit == 0xE)
        v = x * 10.0;
    NPD(0x20) = v;
    if (NPS8(3)) {
        long long q = (long long)v;
        if (q > 0x7FFFFFFFLL)
            q = 0x7FFFFFFFLL;
        else if (q < -0x7FFFFFFFLL)
            q = -0x7FFFFFFFLL;
        *(volatile double *)NP32(0x28) = v;
        *(volatile long long *)NP32(0x2C) = q;
        *(volatile u32 *)NP32(0x30) = (u32)q;
        if (NP8(0x4C))
            ((void (*)(void))NP32(0x50))();
        v = NPD(0x20);
    }
    NPD(0x20) = v;
}
""", src="ui")

wk(0x400617AC, 672, "measure_values_clamp",
   "Numpad callback of the measurement window (skipped while byte 0x42E68BC7 is set): the four s64 values +0x120..+0x138 of block 0x42E68668 are limited to 0..1e9, the window recalculated (window_64_recalc) and the four limited again; the doubles +0x148 to -60..260 and +0x140 to 1..260 (a value is cut to 260 from 260 + 1 ulp up).",
   """
void window_64_recalc(void);
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define LIM 0x1.0400000000001p+8

static void clamp4(void)
{
    u32 o;
    for (o = 0x120; o <= 0x138; o += 8) {
        long long v = M64(o);
        if (v < 0)
            M64(o) = 0;
        else if (v > 1000000000LL)
            M64(o) = 1000000000LL;
    }
}

void measure_values_clamp(void)
{
    double d;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    clamp4();
    window_64_recalc();
    clamp4();
    d = MD(0x148);
    if (d >= LIM)
        MD(0x148) = 260.0;
    else if (d < -60.0)
        MD(0x148) = -60.0;
    d = MD(0x140);
    if (d >= LIM)
        MD(0x140) = 260.0;
    else if (d < 1.0)
        MD(0x140) = 1.0;
}
""", src="ui")
