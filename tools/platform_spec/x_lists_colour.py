# Drawing colour, buffer rectangle copy, the list at SYS+0xFA0 (insert, count, clamps), cursor
# clamps, unit labels, file-exists checks, the per-frame UI update and the draw function of
# window 40.
# exec'd by gen_platform.py


def w(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


SS = """#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
"""

# ---------------------------------------------------------------- colour and buffers
w(0x4002EA38, 76, "draw_colour_set",
  "Colour c = 0xAARRGGBB into the block 0x42E6865C: +0 = A (pixel_put blends when it is non-zero), +1 = R, +2 = G, +3 = B, +8 = c, and +4 = the RGB565 value.", """
void draw_colour_set(u32 c)
{
    volatile unsigned char *p = (volatile unsigned char *)0x42E6865Cu;
    p[1] = c >> 16;
    p[2] = c >> 8;
    p[0] = c >> 24;
    p[3] = c;
    *(volatile u32 *)(p + 8) = c;
    *(volatile unsigned short *)(p + 4) = (((c >> 10) & 0x3F) << 5) | ((c >> 19) << 11) | ((c >> 3) & 0x1F);
}
""", src="display")
w(0x4003436C, 88, "fb_rect_copy_from_e4e84",
  "Copies the rectangle x0 <= x < x1, y0 <= y < y1 (unsigned) of 16-bit pixels, stride 1022, from the buffer 0x420E4E84 to 0x41FB8FE8, then dcache_clean_inv_all_t (with the same r0..r3).", """
void dcache_clean_inv_all_t(u32 a, u32 b, u32 c, u32 d);

void fb_rect_copy_from_e4e84(u32 x0, u32 y0, u32 x1, u32 y1)
{
    u32 y, n;
    for (y = y0; y < y1; y++) {
        if (x0 < x1) {
            volatile unsigned short *s = (volatile unsigned short *)(0x420E4E84u + (x0 + y * 1022u) * 2u);
            volatile unsigned short *d = (volatile unsigned short *)(0x41FB8FE8u + (x0 + y * 1022u) * 2u);
            for (n = x1 - x0; n; n--)
                *d++ = *s++;
        }
    }
    dcache_clean_inv_all_t(x0, y0, x1, y1);
}
""", src="display")

# ---------------------------------------------------------------- the list at SYS+0xFA0
for a, cnt, dec in ((0x400349B4, 0x754, 0), (0x40035A00, 0x756, 0), (0x40036D88, 0x758, 1)):
    w(a, 88 if not dec else 92, "list_fa4_insert_%x" % cnt,
      "Shifts the u16 entries SYS+0xFA6.. up by one (u16 SYS+0x%X%s entries, from the top down), puts v at SYS+0xFA6 and increments the s16 count SYS+0xFA0." % (cnt, " minus 1" if dec else ""), SS + """
void list_fa4_insert_%x(u32 v)
{
    int k;
    for (k = (int)SYS16(0x%X) - %d; k >= 1; k--)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA4 + 2 * k);
    SYS16(0xFA6) = v;
    SS16(0xFA0) = SS16(0xFA0) + 1;
}
""" % (cnt, cnt, dec))
for a in (0x400353A8, 0x40036454, 0x40037880):
    w(a, 76, "list_count_nonzero_%08x" % a,
      "u16 SYS+0x88BE = the number of non-zero bytes among the first s16 SYS+0xFA0 bytes at SYS+0x5DC6 (the store is repeated each step, as in the stock code).", SS + """
void list_count_nonzero_%08x(void)
{
    volatile unsigned char *p = (volatile unsigned char *)(0x40544E68u + 0x5DC6u);
    int n = SS16(0xFA0);
    u32 c = 0;
    for (; n >= 1; n--) {
        SYS16(0x88BE) = c;
        if (*p++)
            c++;
    }
    SYS16(0x88BE) = c;
}
""" % a)
for a in (0x40035248, 0x400362F4, 0x4003771C):
    w(a, 96, "list_cursor_clamp_%08x" % a,
      "s16 SYS+0xFA4 is clamped to 0..s16 SYS+0xFA0 - 1 (always stored); s16 SYS+0xF9E is set to 0 when negative, or to s16 SYS+0xFA2 - 1 when above it.", SS + """
void list_cursor_clamp_%08x(void)
{
    int v = SS16(0xFA4), m;
    if (v < 0) {
        v = 0;
    } else {
        m = SS16(0xFA0) - 1;
        if (m < v)
            v = m;
    }
    SS16(0xFA4) = v;
    v = SS16(0xF9E);
    if (v < 0) {
        SS16(0xF9E) = 0;
    } else {
        m = SS16(0xFA2) - 1;
        if (m < v)
            SS16(0xF9E) = m;
    }
}
""" % a)
w(0x40037418, 72, "list_current_advance",
  "When the u16 SYS+0x3DA52E equals the entry SYS+0xFA6 + 2i: it becomes the next entry (SYS+0xFA8 + 2i) when i < count - 1, else the entry SYS+0xFA4 + 2i.", SS + """
void list_current_advance(int i)
{
    if (SYS16(0xFA6 + 2 * i) != SYS16(0x3DA52E))
        return;
    if (i < SS16(0xFA0) - 1)
        SYS16(0x3DA52E) = SYS16(0xFA8 + 2 * i);
    else
        SYS16(0x3DA52E) = SYS16(0xFA4 + 2 * i);
}
""")
w(0x400374EC, 76, "array_3da548_decimate",
  "For n > 1: keeps every n-th u16 of the array SYS+0x3DA548, packing (word SYS+0x3DA53C) / n of them at its start.", """
void array_3da548_decimate(int n)
{
    volatile unsigned short *d, *s;
    int c;
    if (n <= 1)
        return;
    d = s = (volatile unsigned short *)(0x40544E68u + 0x3DA548u);
    c = (int)SYS32(0x3DA53C) / n;
    for (; c >= 1; c--) {
        *d++ = *s;
        s += n;
    }
}
""")

# ---------------------------------------------------------------- cursor clamps
w(0x400340D4, 84, "cursor_810_clamp",
  "Words SYS+0x810 and +0x814 are kept in 12..(u16 SYS+0xA) - 15 (the first is always stored, the second only when changed).", """
void cursor_810_clamp(void)
{
    int v = (int)SYS32(0x810), m;
    if (v < 12) {
        v = 12;
    } else {
        m = SYS16(0xA) - 15;
        if (m < v)
            v = m;
    }
    SYS32(0x810) = v;
    v = (int)SYS32(0x814);
    if (v < 12) {
        SYS32(0x814) = 12;
    } else {
        m = SYS16(0xA) - 15;
        if (m < v)
            SYS32(0x814) = m;
    }
}
""")
w(0x4003407C, 88, "cursor_808_clamp",
  "Words SYS+0x808 and +0x80C are kept in 12..(u16 SYS+8) - 12 (the first is always stored, the second only when changed).", """
void cursor_808_clamp(void)
{
    int v = (int)SYS32(0x808), m;
    if (v < 12) {
        v = 12;
    } else {
        m = SYS16(8) - 12;
        if (m < v)
            v = m;
    }
    SYS32(0x808) = v;
    v = (int)SYS32(0x80C);
    if (v < 12) {
        SYS32(0x80C) = 12;
    } else {
        m = SYS16(8) - 12;
        if (m < v)
            SYS32(0x80C) = m;
    }
}
""")
w(0x40037A60, 88, "cursor_ac64_clamp",
  "Block 0x42E180C8: words +0xAC64 and +0xAC68 are kept in 2..(u16 +4) - 2.", """
void cursor_ac64_clamp(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E180C8u;
    volatile int *a = (volatile int *)(b + 0xAC64);
    int lim = *(volatile unsigned short *)(b + 4) - 2, x, y;
    if (a[0] < 2)
        a[0] = 2;
    y = a[1];
    x = a[0];
    if (y < 2)
        y = 2;
    if (lim <= x)
        x = lim;
    a[0] = x;
    a[1] = lim > y ? y : lim;
}
""")

# ---------------------------------------------------------------- labels and files
for a, s, txt in ((0x40038DAC, 0x40038E3C, "dB"), (0x40038DEC, 0x4008FFC0, "dBmV")):
    w(a, 64, "level_label_%s" % txt.lower(),
      "0x40038C00(x, y, (u8)c, d0), then 0x4002CE7C((u16)(u16 0x42E68BE4 + x + 4), y, the text '%s' at 0x%08X)." % (txt, s), """
void level_value_draw(u32 x, u32 y, u32 c, double v);
void text_draw(u32 x, u32 y, const char *s);

void level_label_%s(u32 x, u32 y, u32 c, double v)
{
    level_value_draw(x, y, c & 0xFF, v);
    text_draw((*(volatile unsigned short *)0x42E68BE4u + x + 4) & 0xFFFF, y, (const char *)0x%08Xu);
}
""" % (txt.lower(), s))
PATHFN = {0x40034B78: "screenshot_path_make", 0x40035BC4: "waveform_path_make"}
for a, b in ((0x400353F4, 0x40034B78), (0x400364A0, 0x40035BC4)):
    w(a, 64, "file_exists_%08x" % b,
      "0x%08X(a) (builds the path 0x4054D728), then 1 when f_open(read) of it succeeds (the file is closed again), else 0." % b, """
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
void %s(u32 a);

u32 file_exists_%08x(u32 a)
{
    unsigned char fil[0x250];
    u32 r = 0;
    %s(a);
    if (f_open(fil, (const char *)0x4054D728u, 1) == 0) {
        r = 1;
        f_close(fil);
    }
    return r;
}
""" % (PATHFN[b], b, PATHFN[b]))

# ---------------------------------------------------------------- frame update, window 40
w(0x400346C0, 64, "ui_frame_chain",
  "outputs_apply, sys_rect_redraw_unless_t17, acquisition_dispatcher, acquisition_rearm, traces_update, zoom_views_update, 0x40042520, func_40023ea4_or_40024c60, 0x4003117C, 0x40030F8C, 0x40031794, then func_40028364_unless_t17. Most of them do nothing while touch byte +0x17 is set.", """
void sys_rect_redraw_unless_t17(void);
void acquisition_rearm(void);
void traces_update(void);
void zoom_views_update(void);
void func_40042520(void);
void func_40023ea4_or_40024c60(void);
void manual_cursors_draw(void);
void measure_panel_draw(void);
void func_40031794(void);
void func_40028364_unless_t17(void);

void ui_frame_chain(void)
{
    outputs_apply();
    sys_rect_redraw_unless_t17();
    acquisition_dispatcher();
    acquisition_rearm();
    traces_update();
    zoom_views_update();
    func_40042520();
    func_40023ea4_or_40024c60();
    manual_cursors_draw();
    measure_panel_draw();
    func_40031794();
    func_40028364_unless_t17();
}
""")
w(0x40062EE8, 92, "ui_window_40_draw",
  "Draw function of window 40 (block 0x42E68668): +5 = 0; 0x400612FC; unless both +0 and +4 are set, 0x40060FE8, 0x40060DC8, 0x40061F10; then 0x400617AC, 0x400623F4, 0x40062448, 0x40062B1C, 0x40062A18, 0x40062C98, 0x40061194 and func_400606a0_unless_locked.", """
#define CB8(o)  (*(volatile unsigned char *)(0x42E68668u + (o)))
void func_400612fc(void);
void auto_level_search(void);
void func_40060dc8(void);
void func_40061f10(void);
void measure_values_clamp(void);
void levels_update_chain(void);
void func_40062448(void);
void levels_shade_draw(void);
void func_40062a18(void);
void level_columns_draw(void);
void level_columns_draw_b(void);
void func_400606a0_unless_locked(void);

void ui_window_40_draw(void)
{
    CB8(5) = 0;
    func_400612fc();
    if (CB8(0) == 0 || CB8(4) == 0) {
        auto_level_search();
        func_40060dc8();
        func_40061f10();
    }
    measure_values_clamp();
    levels_update_chain();
    func_40062448();
    levels_shade_draw();
    func_40062a18();
    level_columns_draw();
    level_columns_draw_b();
    func_400606a0_unless_locked();
}
""")
w(0x40063714, 92, "fpga_cmd1c_write",
  "Selector 0x1C, then bits 8..15 and 0..7 of the word SYS+0x28C (read before the selector) when SYS+0x275 and +0x274 are set, else two zero bytes.", """
void fpga_cmd1c_write(void)
{
    u32 v = SYS32(0x28C);
    fpga_selector_command_write(0x1C);
    if (SYS8(0x275) && SYS8(0x274)) {
        fpga_data_byte_write((v >> 8) & 0xFF);
        fpga_data_byte_write(v & 0xFF);
    } else {
        fpga_data_byte_write(0);
        fpga_data_byte_write(0);
    }
}
""", src="acquisition")
