# The configuration menus (save / read / power-on configuration, and the list of the five
# configuration slots) and the zoom ratio and zoom-window update of the time base.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define LAY 0x42E64520u              /* UI layout table */
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define B8(a) (*(volatile unsigned char *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

void half_42e68be2_set(u32 a);
void word_42e68c74_set(u32 a);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);
void icon_19c7c_draw(u32 x, u32 y);
"""

wr(0x40028740, 884, "config_menu_draw",
   "Draws the configuration menu (layout +0x68..+0x6E): the box in theme colour SYS+0x3DD550 with a black 2-pixel left edge, then three rows of 0x3C: image 0x4008B2E8 + 0x170 x row, "
   "text entry 83 + row ('Save configuration', 'Read configuration', 'Power-on configuration'), icon 0x40019C7C at x+0x124, and touch button 0x42E61FF8 +0x106 / +0x110 / +0x11A. "
   "A row whose button is pressed becomes the selection (layout byte +0x67 = row + 1); the selected row is shaded with theme colour +8.",
   COM + """
void config_menu_draw(void)
{
    u32 x = H16(LAY + 0x68), y = H16(LAY + 0x6A), w = H16(LAY + 0x6C), h = H16(LAY + 0x6E);
    u32 x2, sl, e;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    x2 = x + 2;
    for (i = 0; i < 3; i++) {
        sl = y + 0x14 + 0x3C * i;
        e = TXT + 0x1A0 * (83 + i);
        draw_colour_set(W32(TH + 4));
        draw_text_shadowed((x2 + 0x14) & 0xFFFF, sl & 0xFFFF, 0x4008B2E8u + 0x170 * i, 2, 2, 0);
        draw_colour_set(W32(TH + 4));
        text_shadow_draw((x2 + 0x41 + SB8(e + 0x194)) & 0xFFFF, (sl + 3 + SB8(e + 0x195)) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        icon_19c7c_draw((x2 + 0x122) & 0xFFFF, sl & 0xFFFF);
        touch_button_set_rect(x2 & 0xFFFF, (sl - 0x14) & 0xFFFF, w, 0x3C, (volatile unsigned char *)(BTN + 0x106 + 0xA * i));
        if (B8(BTN + 0x107 + 0xA * i) || B8(LAY + 0x67) == i + 1) {
            B8(LAY + 0x67) = i + 1;
            draw_colour_set(W32(TH + 8));
            rect_fill(x2 & 0xFFFF, (sl - 0x14) & 0xFFFF, w, 0x3C);
        }
    }
}
""")

wr(0x40028AB4, 1376, "config_slot_menu_draw",
   "Draws the list of the five configuration slots (layout +0x76..+0x7C) beside the configuration menu: the box with a black 2-pixel left edge, then five rows of 0x3C with image "
   "0x4008B738, text entry 86 + row ('Configuration 1' .. '5') and touch button 0x42E61FF8 + 0x124 + 0xA x row. A row is shaded with theme colour +8 while pressed, or when the "
   "menu selection (layout +0x67) is 3 ('Power-on configuration') and it is the power-on slot SYS+0x7C0.",
   COM + """
void config_slot_menu_draw(void)
{
    u32 x = H16(LAY + 0x76), y = H16(LAY + 0x78), w = H16(LAY + 0x7A), h = H16(LAY + 0x7C);
    u32 x2, w2, ys, e;
    int i;

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x, y, w, h);
    draw_colour_set(0);
    rect_fill(x, y, 2, h);
    x2 = x + 2;
    w2 = w - 2;
    for (i = 0; i < 5; i++) {
        ys = (y + 0x14 + 0x3C * i) & 0xFFFF;
        e = TXT + 0x1A0 * (86 + i);
        draw_colour_set(W32(TH + 4));
        draw_text_shadowed((x2 + 0x14) & 0xFFFF, ys, 0x4008B738u, 2, 2, 0);
        draw_colour_set(W32(TH + 4));
        text_shadow_draw((x2 + 0x41 + SB8(e + 0x194)) & 0xFFFF, (ys + 3 + SB8(e + 0x195)) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
        touch_button_set_rect(x2 & 0xFFFF, (ys - 0x14) & 0xFFFF, w2 & 0xFFFF, 0x3C, (volatile unsigned char *)(BTN + 0x124 + 0xA * i));
        if (B8(BTN + 0x125 + 0xA * i) || (B8(SYS + 0x7C0) == i + 1 && B8(LAY + 0x67) == 3)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(x2 & 0xFFFF, (ys - 0x14) & 0xFFFF, w2 & 0xFFFF, 0x3C);
        }
    }
}
""")

wr(0x4004201C, 612, "zoom_ratio_window_update",
   "Unless zoom_mode_check() is set: selects the zoom entry (zoom_entry_select), then with C = u64 SYS+0x5A0, A = s64 SYS+0x5A8, D = SYS+0x554 and P = (1e15 / C) x u16 SYS+0x1C: "
   "n = ceil(A / P) (signed); SYS+0x610 = min(n, 10 x SYS+0x5D4); q = n x D x P / A; the zoom ratio q : D is reduced to SYS+0x61C : SYS+0x618 (the smallest k with q x k divisible "
   "by D, numerator q x k / D); SYS+0x614 = q; u16 SYS+0x54A = SYS+0x610 x u16 SYS+0x5F4. Then the view position s64 SYS+0x578 is clamped so that SYS+0x578 - SYS+0x570 stays "
   "within half the plot width (u16 SYS+8) of samples either side (step s64 SYS+0x590), and the zoom window is set: u16 SYS+0x26 = (SYS+0x570 - SYS+0x578) / step + width / 2, "
   "u16 SYS+0x24 = C x width / step (at least 6). (The symbol table splits this function at 0x40042024, 0x400421A0 and 0x40042200; the entry here covers all pieces.)",
   COM + """
u32 zoom_mode_check(void);
void zoom_entry_select(void);

#define U64(a) (*(volatile unsigned long long *)(a))
#define S64(a) (*(volatile long long *)(a))

void zoom_ratio_window_update(void)
{
    unsigned long long p, k, t;
    long long a, b, c, lo, hi, d, r8;
    u32 den, n, m, q, num, wd, half;

    if (zoom_mode_check())
        return;
    zoom_entry_select();
    p = (1000000000000000ULL / U64(SYS + 0x5A0)) * H16(SYS + 0x1C);
    a = S64(SYS + 0x5A8);
    den = W32(SYS + 0x554);
    W32(SYS + 0x614) = den;
    n = (u32)(a / (long long)p);
    if (a % (long long)p != 0)
        n++;
    m = n;
    t = (unsigned long long)n * den * p;
    q = (u32)((long long)t / a);
    if (W32(SYS + 0x5D4) * 10 < n)
        m = W32(SYS + 0x5D4) * 10;
    if (q == den) {
        k = 1;
        num = 1;
    } else {
        for (k = 1; (long long)((unsigned long long)q * k) % (long long)den != 0; k++)
            ;
        num = (u32)((long long)((unsigned long long)q * k) / (long long)den);
    }
    W32(SYS + 0x61C) = num;
    W32(SYS + 0x618) = (u32)k;
    W32(SYS + 0x610) = m;
    W32(SYS + 0x614) = q;
    H16(SYS + 0x54A) = (unsigned short)(m * H16(SYS + 0x5F4));

    wd = H16(SYS + 8);
    half = wd >> 1;
    b = S64(SYS + 0x590);
    c = S64(SYS + 0x5A0);
    lo = (long long)half * (c - b);
    hi = (long long)half * (b - c);
    d = S64(SYS + 0x578) - S64(SYS + 0x570);
    r8 = S64(SYS + 0x578);
    if (d < lo)
        r8 = lo + S64(SYS + 0x570);
    else if (hi < d)
        r8 = hi + S64(SYS + 0x570);
    S64(SYS + 0x578) = r8;
    H16(SYS + 0x26) = (unsigned short)((u32)((S64(SYS + 0x570) - r8) / S64(SYS + 0x590)) + (u32)((long long)wd / 2));
    n = (u32)((U64(SYS + 0x5A0) * wd) / U64(SYS + 0x590));
    if ((n & 0xFFFF) < 6)
        n = 6;
    H16(SYS + 0x24) = (unsigned short)n;
}
""")
