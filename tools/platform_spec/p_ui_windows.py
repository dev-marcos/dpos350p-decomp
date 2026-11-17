# UI window layer: the table of 14-byte window records at 0x42E64520, the 20-slot window stack
# at 0x42E68BC8, the redraw dispatcher, the per-window redraw wrappers and the animated reveal
# that copies a window from the offscreen buffer into the back buffer. exec'd by gen_platform.py
#
# Window record (DERIVED from the dispatcher and the draw functions): +0 open, +1 dirty,
# +2 "self-contained" (when 0 the window is redrawn whenever a window below it set +4),
# +3 animate on the next redraw, +4 forces redraw of the windows above, +6 x, +8 y, +0xA w, +0xC h
# (halfwords, pixels in the 1022-pixel-wide buffers).


def p(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


UI = """#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)
"""

# ---------------------------------------------------------------- draw target and partial present
p(0x400342DC, 16, "fb_target_back", "Draw target (the pointer at 0x40095498) = the back buffer 0x41BFBF68.", """
void fb_target_back(void)
{
    *(volatile u32 *)0x40095498u = 0x41BFBF68u;
}
""")
p(0x400342EC, 16, "fb_target_offscreen", "Draw target (the pointer at 0x40095498) = the offscreen buffer 0x4233CBBC, used to compose a window before an animated reveal.", """
void fb_target_offscreen(void)
{
    *(volatile u32 *)0x40095498u = 0x4233CBBCu;
}
""")
p(0x400343C4, 104, "fb_present_rect", "Copies rows y0..y1-1, pixels x0..x1 (inclusive), of the back buffer into the scanned-out framebuffer, then flushes the D-cache.", UI + """
void dcache_clean_inv_all_t(void);

void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1)
{
    u32 n = x1 - x0 + 1, rows;
    unsigned short *s, *d;
    if (y0 < y1) {
        s = fb_back + y0 * FB_W + x0;
        d = fb_front + y0 * FB_W + x0;
        for (rows = y1 - y0; rows; rows--) {
            copy_u16(d, s, n);
            d += FB_W;
            s += FB_W;
        }
    }
    dcache_clean_inv_all_t();
}
""")

# ---------------------------------------------------------------- animated reveal
REVEAL = UI + """typedef double (*tanh_fn)(double);
#define TANH_T ((tanh_fn)0x4005F91Du)   /* tanh at 0x4005F91C is Thumb */
void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1);
void ui_window_redraw_dirty(void);
"""
EASE = """
    double t = 0.0, s = 0.2;
    u32 n;
    do {
        double e = TANH_T(t) + 0.02;
        u32 q, i, j;
        t += s;
        s += -0.012;
        if (s < 0.01)
            s = 0.01;
        q = (u32)(e * 10000.0);
        q = q * h / 10000u;
        n = h > q ? q : h;
"""
COPY = """
        if (n) {
            for (i = n; i; i--) {
                for (j = 0; j < w; j++)
                    dst[j] = src[j];
                src += FB_W;
                dst += FB_W;
            }
        }
"""
p(0x4003D594, 280, "ui_reveal_slide_down",
  "Animated reveal of a window rect (record r: x +6, y +8, w +0xA, h +0xC) from the offscreen buffer into the back buffer: at each step the bottom n rows of the new image are copied to the top n rows of the rect and presented, so the window slides down into place. n = min(h, h x floor(10000 x (tanh(t) + 0.02)) / 10000), t from 0 in steps that start at 0.2 and shrink by 0.012 down to 0.01. Ends with ui_window_redraw_dirty.", REVEAL + """
void ui_reveal_slide_down(volatile unsigned short *r)
{
    u32 x = r[3], y = r[4], w = r[5], h = r[6];""" + EASE + """        {
            const unsigned short *src = fb_off + (y + h - n) * FB_W + x;
            unsigned short *dst = fb_back + y * FB_W + x;""" + COPY + """        }
        fb_present_rect(x, y, (x + w) & 0xFFFF, (n + y) & 0xFFFF);
    } while (n < h);
    ui_window_redraw_dirty();
}
""")
p(0x4003D46C, 296, "ui_reveal_bottom_up",
  "As ui_reveal_slide_down, but the top n rows of the new image are copied to the bottom n rows of the rect, so the window rises from the bottom edge.", REVEAL + """
void ui_reveal_bottom_up(volatile unsigned short *r)
{
    u32 x = r[3], y = r[4], w = r[5], h = r[6];""" + EASE + """        {
            const unsigned short *src = fb_off + y * FB_W + x;
            unsigned short *dst = fb_back + (y + h - n) * FB_W + x;""" + COPY + """        }
        fb_present_rect(x, (y + h - n) & 0xFFFF, (x + w) & 0xFFFF, (y + h) & 0xFFFF);
    } while (n < h);
    ui_window_redraw_dirty();
}
""")

# ---------------------------------------------------------------- window stack
p(0x4003CD0C, 56, "ui_window_stack_top_update", "Finds the highest occupied slot of the window stack (slots at 0x42E68BC8+2..+0x15) and stores its index at +1 and its window id at +0; unchanged when the stack is empty.", UI + """
void ui_window_stack_top_update(void)
{
    int i;
    for (i = 19; i >= 0; i--) {
        if (WSTK[2 + i]) {
            WSTK[1] = i;
            WSTK[0] = WSTK[2 + i];
            return;
        }
    }
}
""")
p(0x4003CDFC, 84, "ui_window_open", "Puts window id into the first free stack slot and marks its record open, dirty and animated (+0, +1, +3 = 1); nothing when the 20 slots are full. Then updates the stack top.", UI + """
void ui_window_stack_top_update(void);

void ui_window_open(u32 id)
{
    u32 i;
    for (i = 0; i < 20; i++) {
        if (WSTK[2 + i] == 0) {
            WSTK[2 + i] = id;
            WIN(id)[0] = 1;
            WIN(id)[1] = 1;
            WIN(id)[3] = 1;
            break;
        }
    }
    ui_window_stack_top_update();
}
""")
p(0x4003D6AC, 56, "ui_windows_open_1_5", "Opens windows 1 to 5 in order.", """
void ui_window_open(u32 id);

void ui_windows_open_1_5(void)
{
    ui_window_open(1);
    ui_window_open(2);
    ui_window_open(3);
    ui_window_open(4);
    ui_window_open(5);
}
""")

# ---------------------------------------------------------------- per-window redraw wrappers
# id: (wrapper, draw function, kind) ; kind v = plain (draw straight into the back buffer),
# S = animated with ui_reveal_slide_down, B = animated with ui_reveal_bottom_up, X = window 33.
WINDOWS = {
    1: (0x4003D71C, 0x40019DB8, "v"), 2: (0x4003D734, 0x4001A708, "v"), 3: (0x4003D74C, 0x4001AD50, "v"),
    4: (0x4003D77C, 0x4001C200, "v"), 5: (0x4003D764, 0x4001C348, "v"), 6: (0x4003D794, 0x4001C3BC, "S"),
    7: (0x4003DBD0, 0x40028740, "S"), 8: (0x4003DC08, 0x40028AB4, "S"), 9: (0x4003DC40, 0x400292E4, "S"),
    10: (0x4003D7CC, 0x4001CE30, "S"), 11: (0x4003D81C, 0x4001D608, "S"), 12: (0x4003D86C, 0x4001E8E0, "S"),
    13: (0x4003D8A4, 0x4001EBE4, "B"), 14: (0x4003D8DC, 0x4001F5A8, "B"), 15: (0x4003D914, 0x4001FF74, "B"),
    16: (0x4003D950, 0x40021630, "B"), 17: (0x4003D98C, 0x40021E68, "B"), 18: (0x4003D9C8, 0x40022958, "B"),
    19: (0x4003DA00, 0x40023D04, "v"), 20: (0x4003DA18, 0x400250DC, "S"), 21: (0x4003DA50, 0x40024E38, "S"),
    22: (0x4003DA88, 0x40025D08, "S"), 23: (0x4003DAC0, 0x40025DB4, "v"), 24: (0x4003DAD8, 0x400265BC, "S"),
    25: (0x4003DB10, 0x400262F8, "S"), 26: (0x4003DB48, 0x40027498, "S"), 27: (0x4003DB80, 0x400271B0, "S"),
    28: (0x4003DBB8, 0x40028084, "v"), 29: (0x4003DC78, 0x4002903C, "S"), 30: (0x4003DCB0, 0x4002967C, "S"),
    31: (0x4003DCEC, 0x40029920, "S"), 32: (0x4003DD24, 0x40029BC0, "S"), 33: (0x4003DD5C, 0x4005FF88, "X"),
    34: (0x4003DDB0, 0x4002A184, "v"), 35: (0x4003DDC8, 0x4002A7F4, "v"), 36: (0x4003DDE0, 0x40037AB8, "v"),
    37: (0x4003DDF8, 0x40029EC0, "S"), 38: (0x4003DE34, 0x4002AE60, "v"), 39: (0x4003DE4C, 0x4002B43C, "v"),
    40: (0x4003DE64, 0x40062EE8, "v"), 41: (0x4003DE7C, 0x4002BC40, "S"), 42: (0x4003DEB4, 0x4002C32C, "S"),
}
WRAP = UI + """void fb_target_back(void);
void fb_target_offscreen(void);
void ui_reveal_slide_down(volatile unsigned short *r);
void ui_reveal_bottom_up(volatile unsigned short *r);
"""
for wid, (wa, da, kind) in WINDOWS.items():
    dn = {0x4001C348: "ui_window_05_draw", 0x40062EE8: "ui_window_40_draw"}.get(da, f"func_{da:08x}")
    name = f"ui_window_{wid:02d}_redraw"
    if kind == "v":
        p(wa, 24, name, f"Redraws window {wid}: draw target = back buffer, then the window's draw function 0x{da:08X}.", f"""
void fb_target_back(void);
void {dn}(void);

void {name}(void)
{{
    fb_target_back();
    {dn}();
}}
""")
        continue
    if kind == "X":
        tail = """        fb_target_back();
        if (*(volatile unsigned char *)(0x42E68A88u + 2))
            ui_reveal_slide_down((volatile unsigned short *)WIN(33));
        else
            ui_reveal_bottom_up((volatile unsigned short *)WIN(33));"""
        how = "with ui_reveal_slide_down when the byte 0x42E68A8A is set, else ui_reveal_bottom_up"
    else:
        rv = "ui_reveal_slide_down" if kind == "S" else "ui_reveal_bottom_up"
        tail = f"""        fb_target_back();
        {rv}((volatile unsigned short *)WIN({wid}));"""
        how = f"with {rv}"
    p(wa, 84 if kind == "X" else 56, name,
      f"Redraws window {wid} with the draw function 0x{da:08X}. When anim is set the window is drawn into the offscreen buffer and then revealed into the back buffer {how}; otherwise it is drawn straight into the back buffer.", WRAP + f"""
void {dn}(void);

void {name}(u32 anim)
{{
    if (anim)
        fb_target_offscreen();
    else
        fb_target_back();
    {dn}();
    if (anim) {{
{tail}
    }}
}}
""")

# ---------------------------------------------------------------- dispatcher
protos = "".join(f"void ui_window_{w:02d}_redraw({'void' if k == 'v' else 'u32 anim'});\n" for w, (_, _, k) in WINDOWS.items())
cases = "".join(f"        case {w}: ui_window_{w:02d}_redraw({'' if k == 'v' else 'anim'}); break;\n" for w, (_, _, k) in WINDOWS.items())
p(0x4003CA88, 644, "ui_window_redraw_dirty",
  "Unless the byte 0x42E68BC7 is set, walks the 20 window-stack slots bottom to top and redraws each window (ids 1..42) that is dirty (+1), or that is not self-contained (+2 = 0) while some window below it, or itself, has +4 set; a window with +2 set is redrawn for its own +4 or +1. Clears +1 and +3 before the redraw and passes the old +3 as the animate flag.", UI + protos + """
void ui_window_redraw_dirty(void)
{
    u32 acc = 0, slot;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    for (slot = 0; slot < 20; slot++) {
        u32 id = WSTK[2 + slot];
        volatile unsigned char *w = WIN(id);
        u32 own = w[4], d = w[1], anim, need;
        acc |= own;
        need = (w[2] == 0 ? acc : own) | d;
        anim = w[3];
        if ((need & 0xFF) == 0)
            continue;
        w[1] = 0;
        w[3] = 0;
        switch (id) {
""" + cases + """        default: break;
        }
    }
}
""")
