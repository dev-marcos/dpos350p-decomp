# Drawing primitives (vertical line, rectangle fill and outline, line), tinted glyphs, the message
# box, groups of touch buttons, the marker value list and per-channel min/max.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wb(addr, size, name, desc, body, src="display"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


PR = """#define CB8(o) (*(volatile unsigned char *)(0x42E6865Cu + (o)))
#define CB16(o) (*(volatile unsigned short *)(0x42E6865Cu + (o)))
#define FBP (*(volatile u32 *)0x40095498u)
void pixel_put(u32 x, u32 y);
"""

wb(0x4002EBC8, 132, "vline_draw",
   "Vertical line at x from y0 to y1 inclusive. With a non-zero alpha (byte 0x42E6865C) each pixel goes through pixel_put (blending); otherwise the RGB565 colour (0x42E68660) is stored directly, stride 1022.", PR + """
void vline_draw(u32 x, u32 y0, u32 y1)
{
    int end = y1 + 1;
    u32 y = y0;
    if (CB8(0)) {
        if ((int)y0 >= end)
            return;
        do {
            pixel_put(x, (unsigned short)y);
            y++;
        } while (end > (int)(unsigned short)y);
    } else {
        volatile unsigned short *p;
        int n;
        if ((int)y0 >= end)
            return;
        p = (volatile unsigned short *)(FBP + (x + y0 * 1022u) * 2u);
        n = end - y0;
        do {
            *p = CB16(4);
            p += 1022;
        } while (--n);
    }
}
""")

wb(0x4002ECC4, 204, "rect_fill",
   "Fills w x h pixels at (x, y). With a non-zero alpha each pixel is blended (pixel_blend); otherwise each row is filled with the RGB565 colour (fill_u16), the drawing target pointer read again for every row.", PR + """
void pixel_blend(u32 x, u32 y);
void fill_u16(unsigned short *d, unsigned short v, u32 n);

void rect_fill(u32 x, u32 y, u32 w, u32 h)
{
    if (CB8(0)) {
        int ye = y + h, xe = x + w;
        u32 yy = y;
        if ((int)y >= ye)
            return;
        do {
            if ((int)x < xe) {
                u32 xx = x;
                do {
                    pixel_blend((unsigned short)xx, (unsigned short)yy);
                    xx++;
                } while (xe > (int)(unsigned short)xx);
            }
            yy++;
        } while (ye > (int)(unsigned short)yy);
    } else {
        int ye = (unsigned short)(h + y), n;
        u32 o;
        if ((int)y >= ye)
            return;
        n = ye - y;
        o = y * 511u * 4u;
        do {
            fill_u16((unsigned short *)(FBP + x * 2u + o), CB16(4), w);
            o += 0x7FC;
        } while (--n);
    }
}
""")

wb(0x4002EC4C, 120, "rect_outline_draw",
   "Outline of the w x h rectangle at (x, y): top and bottom rows and the left and right columns, all with rect_fill (the firmware falls through into rect_fill for the last one).", """
void rect_fill(u32 x, u32 y, u32 w, u32 h);

void rect_outline_draw(u32 x, u32 y, u32 w, u32 h)
{
    rect_fill(x, y, w, 1);
    rect_fill(x, (unsigned short)(h + y + 0xFFFF), w, 1);
    rect_fill(x, (unsigned short)(y + 1), 1, (unsigned short)(h - 2));
    rect_fill((unsigned short)(w + x + 0xFFFF), (unsigned short)(y + 1), 1, (unsigned short)(h - 2));
}
""")

wb(0x4002ED90, 216, "line_draw",
   "Line from (x0, y0) to (x1, y1) as vertical segments: slope k = (y1 - y0) * 10000 / (x1 - x0) (0 when x1 = x0, as the sdiv instruction gives), and for each x from x0 towards x1 (inclusive) a vline_draw between the previous and the new y = y0 + (x - x0) * k / 10000.", """
void vline_draw(u32 x, u32 y0, u32 y1);

void line_draw(int x0, int y0, int x1, int y1)
{
    int d = x1 - x0, k = d ? (int)((u32)(y1 - y0) * 10000u) / d : 0;
    int prev = y0, x = x0, y;
    if (x1 >= x0) {
        do {
            y = y0 + (int)((u32)(x - x0) * (u32)k) / 10000;
            if (y >= prev)
                vline_draw((unsigned short)x, (unsigned short)prev, (unsigned short)y);
            else
                vline_draw((unsigned short)x, (unsigned short)y, (unsigned short)prev);
            x++;
            prev = y;
        } while (x1 >= x);
    } else {
        do {
            y = y0 + (int)((u32)(x - x0) * (u32)k) / 10000;
            if (y >= prev)
                vline_draw((unsigned short)x, (unsigned short)prev, (unsigned short)y);
            else
                vline_draw((unsigned short)x, (unsigned short)y, (unsigned short)prev);
            x--;
            prev = y;
        } while (x >= x1);
    }
}
""")

wb(0x4002D724, 188, "text_d6f8_shadow_draw",
   "Like text_shadow_draw, for the text routine 0x4002D6F8 (args a, b and the byte e passed on): an n x m block of copies offset by 1..n / 1..m in colour alpha << 24, then the colour (word 0x42E68664) is restored and the text is drawn at (x, y).", """
void draw_colour_set(u32 c);
void func_4002d82c_flag0(u32 a, u32 b, u32 c, u32 d, u32 e);

void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m,
                           unsigned char alpha)
{
    u32 saved = *(volatile u32 *)0x42E68664u, xx, yy, i, j;
    draw_colour_set((u32)alpha << 24);
    if (n) {
        xx = (unsigned short)x + 1;
        for (i = n; i; i--, xx++) {
            yy = (unsigned short)y + 1;
            for (j = m; j; j--, yy++)
                func_4002d82c_flag0((unsigned short)xx, (unsigned short)yy, a, b, e);
        }
        y = (unsigned short)y;
    }
    draw_colour_set(saved);
    func_4002d82c_flag0((unsigned short)x, y, a, b, e);
}
""")

wb(0x4002E878, 436, "font_char_tint_draw",
   "Like font_char_draw, but each non-zero glyph byte v is an intensity: the drawing colour becomes v * (R, G, B) / 255 of the colour at 0x42E6865C (bytes +1..+3 and the RGB565 +4 rewritten) before pixel_put. The colour word +8 is restored at the end.", PR + """
void draw_colour_set(u32 c);
#define FONT (*(volatile u32 *)0x42E68C74u)
#define H16(a) (*(volatile unsigned short *)(a))

void font_char_tint_draw(u32 x, u32 y, u32 ch)
{
    int r0 = CB8(1), g0 = CB8(2), b0 = CB8(3);
    u32 saved = *(volatile u32 *)0x42E68664u;
    const unsigned char *bits = *(const unsigned char * volatile *)(FONT + 0x4BC);
    unsigned short r, k;
    ch = (unsigned short)ch;
    for (r = 0; r < H16(FONT + 4) - 1; r++) {
        u32 o = H16(FONT + 2) * (r + 1) + H16(FONT + 2 * ch + 0xA);
        u32 xx = (unsigned short)x, yy = r + (unsigned short)y;
        for (k = 0; k < H16(FONT + 2 * ch + 0x262); k++, o++, xx++) {
            int v = bits[(unsigned short)o], cr, cg, cb;
            if (v == 0)
                continue;
            cr = v * r0 / 255;
            cg = v * g0 / 255;
            cb = v * b0 / 255;
            CB8(3) = cb;
            CB8(1) = cr;
            CB8(2) = cg;
            CB16(4) = (((unsigned char)cg >> 2) << 5) | (((unsigned char)cr >> 3) << 11) | ((unsigned char)cb >> 3);
            pixel_put((unsigned short)xx, (unsigned short)yy);
        }
    }
    draw_colour_set(saved);
}
""")

wb(0x40034134, 336, "message_box_draw",
   "Draws a message box with the text s near (x, y): width = text width (0x4002D164) + 50, height 50; align 0 centres it above the point (y - 25), 2 puts it to the upper left (x - w, y - 50), both clamped at 0; 1 and others leave (x, y). Saves the area to 0x420E4E84 first, stores the box in 0x42E68C38, draws a black box with a 0x282828 inside and the text (colour word 0x409223BC, shadow 1 x 2), then presents the rectangle.", """
void word_42e68c74_set(u32 a);
void half_42e68be2_set(u32 a);
int func_4002d164(const char *s);
void fb_rect_copy_to_e4e84(u32 x0, u32 y0, u32 x1, u32 y1);
void fb_target_back(void);
void draw_colour_set(u32 c);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha);
void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1);

void message_box_draw(int x, int y, u32 text, int align)
{
    volatile unsigned short *b = (volatile unsigned short *)0x42E68C38u;
    const char *s = (const char *)text;
    int w;
    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(2);
    w = (unsigned short)(func_4002d164(s) + 0x32);
    if (align == 0) {
        y -= 0x19;
        x -= w / 2;
    } else if (align == 2) {
        x -= w;
        y -= 0x32;
    }
    if (align == 0 || align == 2) {
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
    }
    b[3] = 0x32;
    b[0] = x;
    b[1] = y;
    b[2] = w;
    fb_rect_copy_to_e4e84((unsigned short)x, (unsigned short)y, (unsigned short)(w + x + 1), (unsigned short)(y + 0x33));
    fb_target_back();
    draw_colour_set(0);
    rect_fill((unsigned short)x, (unsigned short)y, w, 0x32);
    draw_colour_set(0x282828);
    rect_fill((unsigned short)(x + 1), (unsigned short)(y + 1), (unsigned short)(w - 2), 0x30);
    draw_colour_set(*(volatile u32 *)0x409223BCu);
    text_shadow_draw((unsigned short)(x + 0x19), (unsigned short)(y + 0x11), s, 1, 2, 0);
    fb_present_rect((unsigned short)x, (unsigned short)y, (unsigned short)(w + x), (unsigned short)(y + 0x32));
}
""", src="ui")

# ---------------------------------------------------------------- groups of touch buttons
TB = 0x42E61FF8
TBH = """void touch_button_update(volatile unsigned char *b);
#define TBU(o) touch_button_update((volatile unsigned char *)(0x42E61FF8u + (o)))
"""
A_ = [0x624, 0x62E, 0x638, 0x642, 0x656, 0x660, 0x66A, 0x674, 0x67E]
B_ = list(range(0x688, 0x715, 10))
wb(0x4002F968, 316, "touch_buttons_6xx_update",
   "touch_button_update for the button at 0x42E62644, then for 9 buttons (0x42E6261C..) when byte 0x42E3F990 is 0, otherwise for 15 others (0x42E62680..).", TBH + """
void touch_buttons_6xx_update(void)
{
    TBU(0x64C);
    if (*(volatile unsigned char *)(0x42E3F984u + 0xC) == 0) {
%s
    } else {
%s
    }
}
""" % ("\n".join("        TBU(0x%X);" % o for o in A_), "\n".join("        TBU(0x%X);" % o for o in B_)), src="ui")

for a, size, base, tail in ((0x4002FAAC, 148, 0x728, [0x7BE, 0x7C8, 0x7D2, 0x7DC, 0x7E6, 0x7F0]),
                            (0x4002FB98, 152, 0x840, [0x8D6, 0x8E0, 0x8EA, 0x8F4, 0x8FE, 0x908]),
                            (0x4002FC38, 148, 0x91C, [0x9B2, 0x9BC, 0x9C6, 0x9D0, 0x9DA, 0x9E4])):
    wb(a, size, "touch_buttons_%x_update" % base,
       "touch_button_update for the first u16 SYS+0xF9A buttons from 0x%08X (10 bytes each, a byte counter) and six more buttons." % (TB + base), TBH + """
void touch_buttons_%x_update(void)
{
    unsigned char i;
    for (i = 0; i < SYS16(0xF9A); i++)
        TBU(0x%X + i * 10);
%s
}
""" % (base, base, "\n".join("    TBU(0x%X);" % o for o in tail)), src="ui")

wb(0x4002FDCC, 112, "touch_buttons_a20_update",
   "touch_button_update for 10 buttons from 0x42E62A18 and six more.", TBH + """
void touch_buttons_a20_update(void)
{
    int i;
    for (i = 0; i < 10; i++)
        TBU(0xA20 + i * 10);
%s
}
""" % "\n".join("    TBU(0x%X);" % o for o in (0xA84, 0xA8E, 0xA98, 0xAA2, 0xAAC, 0xAB6)), src="ui")

# ---------------------------------------------------------------- data
wb(0x40030060, 136, "marker_value_add",
   "Adds v (negated when mode is 1) to the 24 s16 values at SYS+0x880 unless it is already there: increments the counter SYS+0x800 (mode 0) or SYS+0x801 (other modes), then stores v in the first zero slot (if any).", """
void marker_value_add(u32 mode, int v)
{
    volatile short *t = (volatile short *)(0x40544E68u + 0x880u);
    int i;
    if (mode == 1)
        v = -v;
    for (i = 0; i < 24; i++)
        if (t[i] == v)
            return;
    if (mode == 0)
        SYS8(0x800)++;
    else
        SYS8(0x801)++;
    for (i = 0; i < 24; i++) {
        if (t[i] == 0) {
            t[i] = v;
            return;
        }
    }
}
""", src="acquisition")

wb(0x40032A58, 228, "channel_minmax_stats",
   "For channel ch (0 needs SYS+0x3E, 1 needs SYS+0x102): over the samples (u16 array at word SYS+0x7B4 + 4ch) from h = (SYS+0x2B4 - SYS+0x2B0) / 2 up to SYS+0x2B4 - h, the maximum and minimum, their mean and the range, stored in SYS+0xB60/B64/B6C/B68 (ch 0) or SYS+0xB88/B8C/B94/B90.", """
void channel_minmax_stats(u32 ch)
{
    volatile unsigned short *p = (volatile unsigned short *)SYS32(0x7B4 + 4 * ch);
    u32 omax, omin, omid, orng;
    int h, end, i, mx = 0, mn = 0xFFFFFF;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        omax = 0xB60; omin = 0xB64; omid = 0xB6C; orng = 0xB68;
    } else {
        if (ch == 1 && SYS8(0x102) == 0)
            return;
        omax = 0xB88; omin = 0xB8C; omid = 0xB94; orng = 0xB90;
    }
    h = (u32)(SYS32(0x2B4) - SYS32(0x2B0)) >> 1;
    end = SYS32(0x2B4) - h;
    for (i = h; i < end; i++) {
        int v = p[i];
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
    }
    SYS32(omax) = mx;
    SYS32(omin) = mn;
    SYS32(omid) = (mn + mx) / 2;
    SYS32(orng) = mx - mn;
}
""", src="acquisition")
