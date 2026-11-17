# Trace sample scaling for one segment and the two level-column graphs.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wl(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wl(0x4005F778, 352, "trace_scale_segment",
   "Scales trace tr's raw samples start..start+len (u16, buffer 0x42AC7CF4 + sel * 0x62250 with sel = mode_not_0_or_2(tr)) into screen samples (u16, 0x411AF3E8 + tr * 0x927C0): y = base + ((off + ((x - off - pos) * gain >> 10)) * mul >> 12), clamped to min..max. k = SYS+0x1C0; gain / pos / offset are SYS+0xE4 / +0xE8 / +0x60 (sel 0) or +0x1A4 / +0x1A8 / +0x120; off = k * offset, pos = k * pos. Full screen (SYS+0x2AA = 0): mul 0x1000, range 4..height-5 (height = SYS+0xA); split screen: the lower third (mul 0x555, base 2h/3, mode_not_0_or_1(tr) = 0) or the upper two thirds (mul 0xAAB). min, max and base are scaled by k.",
   """
u32 mode_not_0_or_2(u32 v);
u32 mode_not_0_or_1(u32 v);

void trace_scale_segment(u32 tr, u32 start, u32 len)
{
    int end = (int)(len + start + 1);
    u32 sel = mode_not_0_or_2(tr);
    u32 k = SYS32(0x1C0), gain, pos, off, mul;
    int mx, mn, base;
    if (sel == 0) {
        gain = SYS32(0xE4);
        pos = SYS32(0xE8);
        off = SYS32(0x60);
    } else {
        gain = SYS32(0x1A4);
        pos = SYS32(0x1A8);
        off = SYS32(0x120);
    }
    off = k * off;
    if (SYS8(0x2AA) == 0) {
        mx = (int)SYS16(0xA) - 5;
        mul = 0x1000;
        base = 0;
        mn = 4;
    } else if (mode_not_0_or_1(tr) == 0) {
        u32 v = SYS16(0xA);
        int t = (int)(2 * v) / 3;
        mx = t + (int)(v / 3) - 5;
        mn = t + 4;
        mul = 0x555;
        base = t;
    } else {
        u32 v = SYS16(0xA);
        mx = (int)(2 * v) / 3 - 5;
        mul = 0xAAB;
        base = 0;
        mn = 4;
    }
    k = SYS32(0x1C0);
    mx = (int)((u32)mx * k);
    mn = (int)((u32)mn * k);
    base = (int)((u32)base * k);
    pos = k * pos;
    if ((int)start < end) {
        volatile unsigned short *s = (volatile unsigned short *)(0x42AC7CF4u + sel * 0x62250u) + start;
        volatile unsigned short *d = (volatile unsigned short *)(0x411AF3E8u + tr * 0x927C0u) + start;
        int n = end - (int)start;
        do {
            int t = (int)(((u32)*s++ - off - pos) * gain);
            int a = (int)((off + (u32)(t >> 10)) * mul);
            int y = (int)((u32)base + (u32)(a >> 12));
            if (mx <= y)
                y = mx;
            if (mn >= y)
                y = mn;
            *d++ = (unsigned short)y;
        } while (--n);
    }
}
""")

COLS = '''void draw_colour_set(u32 c);
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define COLOUR16 (*(volatile unsigned short *)0x42E68660u)
#define SCREEN (*(volatile u32 *)0x40095498u)
'''

wl(0x40061194, 332, "level_columns_draw_b",
   "Second level-column graph (enabled by byte 0x42E6866A, skipped while 0x42E68BC7 is set): for each of the M+0xEC columns, the rows from the height M+0xF0 up to the column top (u16 0x42E61000 + 2col) get the colour of table 0x42E583A0[(int)(i * 510.0 / (height - M+0x174))] (i counting up from the bottom) in the buffer 0x425948F4 (width M+0xEC); then the M+0xF0 rows of the buffer are copied to the screen *0x40095498 at row M+0xE8 + r (stride 1022 pixels), column M+0xE4. M = 0x42E68668.",
   COLS + """
#define TOP16(c) (*(volatile unsigned short *)(0x42E61000u + 2 * (c)))
#define TBL(i) (*(volatile u32 *)(0x42E583A0u + 4 * (i)))
#define BUF ((volatile unsigned short *)0x425948F4u)

void level_columns_draw_b(void)
{
    int col, r;
    u32 h;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (MB8(2) == 0)
        return;
    h = MB32(0xF0);
    for (col = 0; col < (int)MB32(0xEC); col++) {
        int top = TOP16(col), y = (int)h, i = 0;
        double k = 510.0 / (double)(u32)(h - MB32(0x174));
        if (y >= top) {
            do {
                draw_colour_set(TBL((int)((double)i * k)));
                i++;
                BUF[MB32(0xEC) * (u32)y + (u32)col] = COLOUR16;
                y--;
            } while (y >= top);
        }
    }
    for (r = 0; r < (int)MB32(0xF0); r++)
        copy_u16((unsigned short *)(SCREEN + ((u32)r + MB32(0xE8)) * 2044u + MB32(0xE4) * 2u),
                 (const unsigned short *)(BUF + MB32(0xEC) * (u32)r), MB32(0xEC));
}
""")

wl(0x40062C98, 352, "level_columns_draw",
   "Level-column graph (enabled by byte 0x42E68669, skipped while 0x42E68BC7 is set): for each of the M+0xDC columns and each step i from the height h = M+0xE0 down to the column top (s16 0x42E60008 + 2col), the colour of table 0x42E56C30[(int)(i * 1000.0 / h)] is set and M+0x184 pixels of the column are painted upwards from the bottom row of the buffer 0x42B8C194 (width M+0xDC; the start does not move with i, as in the stock code); then the M+0xE0 rows are copied to the screen *0x40095498 at row M+0xD8 + r (stride 1022 pixels), column M+0xD4. M = 0x42E68668.",
   COLS + """
#define TOP16(c) (*(volatile short *)(0x42E60008u + 2 * (c)))
#define TBL(i) (*(volatile u32 *)(0x42E56C30u + 4 * (i)))
#define BUF ((volatile unsigned short *)0x42B8C194u)

void level_columns_draw(void)
{
    u32 h0 = MB32(0xE0);
    double k;
    int col, r;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (MB8(1) == 0)
        return;
    k = 1000.0 / (double)h0;
    for (col = 0; col < (int)MB32(0xDC); col++) {
        int top = TOP16(col), y = (int)h0, i = 0;
        if (y >= top) {
            do {
                u32 w;
                int n;
                volatile unsigned short *p;
                draw_colour_set(TBL((int)((double)i * k)));
                w = MB32(0xDC);
                p = BUF + (w * MB32(0xE0) + (u32)col);
                n = (int)MB32(0x184);
                if (n > 0) {
                    unsigned short c = COLOUR16;
                    do {
                        *p = c;
                        p -= w;
                    } while (--n);
                }
                y--;
                i++;
            } while (y >= top);
        }
    }
    for (r = 0; r < (int)MB32(0xE0); r++)
        copy_u16((unsigned short *)(SCREEN + ((u32)r + MB32(0xD8)) * 2044u + MB32(0xD4) * 2u),
                 (const unsigned short *)(BUF + MB32(0xDC) * (u32)r), MB32(0xDC));
}
""")

wl(0x40060FE8, 420, "auto_level_search",
   "Channel 1 level search (DERIVED; skipped while byte 0x42E68BC7 is set): channel 1 on (SYS+0x3E, +0x3F = 1, SYS+0x102 = 0), offset SYS+0x60 = 250, SYS+0x46 = 0, SYS+0x54F = 1, SYS+0x275 = 1, SYS+0x70D = 0, SYS+0x660 = 0, range SYS+0x588 = 0xB, byte 0x42E68BC6 = 1. Then it applies the outputs and runs one acquisition per step, lowering the level byte SYS+0x41 (down to 0) while the measured word SYS+0xB30 is above 0.9 * height (SYS+0xA), raising it (up to 7) while it is below 0.2 * height. Afterwards the double 0x42E22DC0 = s64 SYS+0x910, fft_input_prepare_b runs and the two level-column buffers (0x42B8C194 width M+0xDC, rows M+0xE0; 0x425948F4 width M+0xEC, rows M+0xF0; when enabled by M+1 / M+2) are scrolled up by M+0x184 rows. M = 0x42E68668.",
   COLS + """
void outputs_apply(void);
void acquisition_dispatcher(void);
void fft_input_prepare_b(void);

static void scroll_rows(u32 buf, u32 ow, u32 oh)
{
    int i;
    if ((int)(MB32(oh) - MB32(0x184)) < 0)
        return;
    i = 0;
    do {
        u32 w = MB32(ow);
        copy_u16((unsigned short *)(buf + 2 * (w * (u32)i)), (const unsigned short *)(buf + 2 * (w * (MB32(0x184) + (u32)i))), w);
        i++;
    } while ((int)(MB32(oh) - MB32(0x184)) >= i);
}

void auto_level_search(void)
{
    u32 h, hi, lo, v, l;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS8(0x3F) = 1;
    SYS32(0x60) = 0xFA;
    SYS8(0x46) = 0;
    SYS8(0x54F) = 1;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 1;
    SYS8(0x275) = 1;
    h = SYS16(0xA);
    SYS8(0x70D) = 0;
    SYS8(0x660) = 0;
    SYS8(0x588) = 0xB;
    hi = (u32)((double)h * 0.9);
    lo = (u32)((double)h * 0.2);
    for (;;) {
        outputs_apply();
        acquisition_dispatcher();
        v = SYS32(0xB30);
        if (hi < v) {
            l = SYS8(0x41);
            if (l == 0)
                break;
            SYS8(0x41) = l - 1;
        } else if (v < lo) {
            l = SYS8(0x41);
            if ((int)l >= 7)
                break;
            SYS8(0x41) = l + 1;
        } else {
            break;
        }
    }
    *(volatile double *)0x42E22DC0u = (double)*(volatile long long *)(0x40544E68u + 0x910);
    fft_input_prepare_b();
    if (MB8(1))
        scroll_rows(0x42B8C194u, 0xDC, 0xE0);
    if (MB8(2))
        scroll_rows(0x425948F4u, 0xEC, 0xF0);
}
""")
