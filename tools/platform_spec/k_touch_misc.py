# Touch-control hit tests and drag helpers (0x40069084..0x400695CC), complex-vector helpers,
# LRADC set-up, strcmp and the USB disk geometry update. exec'd by gen_platform.py


def k(addr, size, name, desc, body, src="touch"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


T = """#define TP      ((volatile short *)0x42E68BB0u)            /* touch block: x0, y0, x1, y1 (s16) */
#define TS32(o) (*(volatile u32 *)(0x42E68998u + (o)))     /* touch-control state */
#define TS8     (*(volatile unsigned char *)0x42E68998u)   /* control code of the current touch */
"""


def drag(addr, name, flag, what):
    k(addr, 96, name, f"Drag distance (x1 - x0, y1 - y0) into touch state +0x30/+0x34, divided by 10 when {what} is set.", T + f"""
void {name}(void)
{{
    int dx = TP[2] - TP[0], dy = TP[3] - TP[1];
    if ({flag}) {{
        dx /= 10;
        dy /= 10;
    }}
    TS32(0x34) = dy;
    TS32(0x30) = dx;
}}
""")


drag(0x40069084, "touch_drag_delta_b", "SYS8(0x671)", "SYS_INFO+0x671")
drag(0x400691D4, "touch_drag_delta_c", "*(volatile unsigned char *)(0x42E68668u + 3)", "byte 0x42E6866B")
drag(0x40069324, "touch_drag_delta_d", "*(volatile unsigned char *)(0x42E180C8u + 0x46)", "byte 0x42E1810E")

k(0x400690E4, 108, "touch_hit_pair_a",
  "Horizontal distance from the touch x to two marks (0x42E68668: +0x5C/+0x60 plus offset +0xB0, as u16); a distance above the limit (touch state +0x74) becomes 0xFFFF. Stored at +0x6C/+0x70.", T + """
void touch_hit_pair_a(void)
{
    volatile u32 *b = (volatile u32 *)0x42E68668u;
    int x = TP[2];
    int a = (unsigned short)(b[0x5C / 4] + b[0xB0 / 4]);
    int c = (unsigned short)(b[0x60 / 4] + b[0xB0 / 4]);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TS32(0x74) < d0) d0 = 0xFFFF;
    if (TS32(0x74) < d1) d1 = 0xFFFF;
    TS32(0x6C) = d0;
    TS32(0x70) = d1;
}
""")
k(0x40069150, 132, "touch_pick_a",
  "Control code of a touch: 0 when the second point is (0, 0); otherwise, when 0x42E68668+0x58 is set, the nearer of the two marks of touch_hit_pair_a (0x10 or 0x11) if it is within the limit +0x74, else 0.", T + """
void touch_hit_pair_a(void);

void touch_pick_a(void)
{
    u32 code = 0, best = 0x7FFFFFFF, res = 0;
    if (TP[2] == 0 && TP[3] == 0) {
        TS8 = 0;
        return;
    }
    if (*(volatile u32 *)(0x42E68668u + 0x58) != 0) {
        touch_hit_pair_a();
        if (TS32(0x6C) < 0x7FFFFFFF) {
            best = TS32(0x6C);
            code = 0x10;
        }
        if (TS32(0x70) < best) {
            code = 0x11;
            best = TS32(0x70);
        }
    }
    if (best < TS32(0x74))
        res = code;
    TS8 = res;
}
""")
k(0x40069234, 108, "touch_hit_pair_b",
  "Like touch_hit_pair_a for the marks at 0x42E180C8+0xAC64/+0xAC68 plus the u16 at 0x42E180C8 (32-bit sums, no truncation), limit +0x60, results +0x64/+0x68.", T + """
void touch_hit_pair_b(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E180C8u;
    int x = TP[2];
    u32 o = *(volatile unsigned short *)b;
    int a = (int)(*(volatile u32 *)(b + 0xAC64) + o);
    int c = (int)(*(volatile u32 *)(b + 0xAC68) + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TS32(0x60) < d0) d0 = 0xFFFF;
    if (TS32(0x60) < d1) d1 = 0xFFFF;
    TS32(0x64) = d0;
    TS32(0x68) = d1;
}
""")
k(0x400692A0, 132, "touch_pick_b",
  "Like touch_pick_a with touch_hit_pair_b, enable byte 0x42E22D28, codes 0x0E / 0x0F and limit +0x60.", T + """
void touch_hit_pair_b(void);

void touch_pick_b(void)
{
    u32 code = 0, best = 0x7FFFFFFF, res = 0;
    if (TP[2] == 0 && TP[3] == 0) {
        TS8 = 0;
        return;
    }
    if (*(volatile unsigned char *)0x42E22D28u != 0) {
        touch_hit_pair_b();
        if (TS32(0x64) < 0x7FFFFFFF) {
            best = TS32(0x64);
            code = 0x0E;
        }
        if (TS32(0x68) < best) {
            code = 0x0F;
            best = TS32(0x68);
        }
    }
    if (best < TS32(0x60))
        res = code;
    TS8 = res;
}
""")

HX = """
void {name}(void)
{{
    int x = TP[2];
    u32 o = SYS16(4);
    int a = (unsigned short)({w0} + o);
    int c = (unsigned short)({w1} + o);
    u32 d0 = x >= a ? (u32)(x - a) : (u32)(a - x);
    u32 d1 = x >= c ? (u32)(x - c) : (u32)(c - x);
    if (TS32(4) < d0) d0 = 0xFFFF;
    if (TS32(4) < d1) d1 = 0xFFFF;
    if (SYS8(0x2AA) && TP[3] < (int)(SYS16(6) + ((SYS16(0xA) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TS32({o1}) = d1;
    TS32({o0}) = d0;
}}
"""
k(0x40069384, 164, "touch_hit_cursor_x",
  "Horizontal distance from the touch x to the two cursors SYS_INFO+0x808/+0x80C (plus the x origin SYS+4, as u16), limited by touch state +4; ignored (0xFFFF) above the top third of the plot when SYS+0x2AA is set. Results +0x48/+0x4C.",
  T + HX.format(name="touch_hit_cursor_x", w0="SYS32(0x808)", w1="SYS32(0x80C)", o0="0x48", o1="0x4C"))
k(0x40069524, 172, "touch_hit_x_b",
  "As touch_hit_cursor_x for the marks at SYS_INFO+0x3DA534/+0x3DA538. Results +0x58/+0x5C.",
  T + HX.format(name="touch_hit_x_b", w0="SYS32(0x3DA534)", w1="SYS32(0x3DA538)", o0="0x58", o1="0x5C"))
k(0x40069428, 252, "touch_hit_cursor_y",
  "Vertical distance from the touch y to the cursors SYS_INFO+0x810/+0x814 (plus the y origin SYS+6, as u16); with SYS+0x2AA set the cursor is mapped as 2/3 of its value below the top third, and touches above that third are ignored. Limit touch state +4; results +0x50/+0x54.", T + """
static unsigned short cur_y(u32 v)
{
    if (SYS8(0x2AA))
        return (int)(v << 1) / 3 + (SYS16(6) + ((SYS16(0xA) * 0xAAABu) >> 17));
    return v + SYS16(6);
}

void touch_hit_cursor_y(void)
{
    int y = TP[3];
    int a = cur_y(SYS32(0x810));
    u32 d0 = y >= a ? (u32)(y - a) : (u32)(a - y);
    int c = cur_y(SYS32(0x814));
    u32 d1 = y >= c ? (u32)(y - c) : (u32)(c - y);
    if (TS32(4) < d0) d0 = 0xFFFF;
    if (TS32(4) < d1) d1 = 0xFFFF;
    if (SYS8(0x2AA) && y < (int)(SYS16(6) + ((SYS16(0xA) * 0xAAABu) >> 17)))
        d0 = d1 = 0xFFFF;
    TS32(0x54) = d1;
    TS32(0x50) = d0;
}
""")

# ---------------------------------------------------------------- misc
k(0x40066538, 76, "cvec_magnitude", "dst[i] = sqrtf-by-double(re^2 + im^2) for n complex floats (n <= 0: nothing).", """
typedef double (*sqrt_fn)(double);
#define SQRT_T ((sqrt_fn)0x4003DF75u)   /* sqrt at 0x4003DF74 is Thumb */

void cvec_magnitude(const volatile float *src, volatile float *dst, int n)
{
    for (; n >= 1; n--) {
        float re = src[0], im = src[1];
        float m = re * re + im * im;
        src += 2;
        *dst++ = (float)SQRT_T((double)m);
    }
}
""", src="dsp")
k(0x40066584, 36, "cmul", "Complex float product (a.re b.re - a.im b.im, a.re b.im + a.im b.re), returned in s0/s1.", """
typedef struct { float re, im; } cfloat;

cfloat cmul(cfloat a, cfloat b)
{
    cfloat r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
}
""", src="dsp")
k(0x40068E94, 112, "lradc_init", "LRADC control 0x01C22800: clear bits 12-13, set bit 13, clear bits 4-5, rewrite, then set bit 0 (enable), each a read-modify-write.", """
u32 readl_c(u32 addr);
void writel_c(u32 addr, u32 v);

void lradc_init(void)
{
    const u32 r = 0x01C22800u;
    writel_c(r, readl_c(r) & ~0x3000u);
    writel_c(r, readl_c(r) | 0x2000u);
    writel_c(r, readl_c(r) & ~0x30u);
    writel_c(r, readl_c(r));
    writel_c(r, readl_c(r) | 1u);
}
""", src="platform")
k(0x40068F78, 160, "strcmp", "Byte-wise string compare (the stock code compares a word at a time when it can); returns the difference of the first unequal unsigned bytes.", """
int strcmp(const char *a, const char *b)
{
    const volatile unsigned char *p = (const volatile unsigned char *)a, *q = (const volatile unsigned char *)b;
    u32 c, d;
    do {
        c = *p++;
        d = *q++;
    } while (c >= 1 && c == d);
    return (int)(c - d);
}
""", src="libc")
k(0x4006A98C, 120, "usb_disk_geometry_update",
  "USB disk geometry: 0x42E68D0C = sector size (0x4009548C); when the SD state is 1 the byte capacity 0x42E68C60 = sectors (0x42E68A18+4) x size; 0x42E68D10 = capacity / size (64-bit).", """
unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d);

void usb_disk_geometry_update(void)
{
    u32 ss = *(volatile u32 *)0x4009548Cu;
    volatile unsigned long long *cap = (volatile unsigned long long *)0x42E68C60u;
    *(volatile u32 *)0x42E68D0Cu = ss;
    if (*(volatile u32 *)0x40095490u == 1)
        *cap = (unsigned long long)*(volatile u32 *)(0x42E68A18u + 4) * ss;
    *(volatile u32 *)0x42E68D10u = (u32)__aeabi_uldivmod(*cap, ss);
}
""", src="usb")
