# Touch buttons: the 10-byte button records of the table 0x42E61FF8 (state, touch byte, rectangle),
# the per-screen group updaters, the window touch handling, three release handlers, and some
# loose helpers (entry-pad modes 4..6 and sign, layer clear, sine table, small gates).
# exec'd by gen_platform.py


def w(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


TOUCH = """#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */
#define TCH16(o) (*(volatile short *)(0x42E68BB0u + (o)))
"""
BTN = 0x42E61FF8
BTNH = """#define BTN8(o)  (*(volatile unsigned char *)(0x42E61FF8u + (o)))   /* button table */
"""
WINH = """#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WH(p, o) (*(volatile unsigned short *)((p) + (o)))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
"""

# ---------------------------------------------------------------- button record primitives
w(0x4002F2BC, 104, "touch_button_update",
  "Button record b (+0 state, +1 touch byte, +2/+4 x0/y0, +6/+8 x1/y1, all u16): when the current touch point (s16 at 0x42E68BB0+4/+6) lies strictly inside the rectangle, +1 = touch byte +0xD, and +0 = 1 or 2 when the touch state +0xC is 1 (press) or 2 (release).", TOUCH + """
void touch_button_update(volatile unsigned char *b)
{
    int x = TCH16(4), y;
    if (!((int)*(volatile unsigned short *)(b + 2) < x))
        return;
    if (!(x < (int)*(volatile unsigned short *)(b + 6)))
        return;
    y = TCH16(6);
    if (!((int)*(volatile unsigned short *)(b + 4) < y))
        return;
    if (!(y < (int)*(volatile unsigned short *)(b + 8)))
        return;
    b[1] = TCH8(0xD);
    if (TCH8(0xC) == 1)
        b[0] = 1;
    if (TCH8(0xC) == 2)
        b[0] = 2;
}
""")
w(0x4002F160, 32, "touch_button_set_rect", "Button rectangle from x, y, w, h: +2 = x, +6 = x + w, +4 = y, +8 = y + h (16 bit); the record is the fifth (stack) argument.", """
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b)
{
    *(volatile unsigned short *)(b + 2) = x;
    *(volatile unsigned short *)(b + 6) = x + wd;
    *(volatile unsigned short *)(b + 4) = y;
    *(volatile unsigned short *)(b + 8) = y + ht;
}
""")
w(0x4002F180, 20, "touch_button_disable", "Moves the button off screen: +2, +6 and +4 = 10000 (+8 is left alone).", """
void touch_button_disable(volatile unsigned char *b)
{
    *(volatile unsigned short *)(b + 2) = 10000;
    *(volatile unsigned short *)(b + 6) = 10000;
    *(volatile unsigned short *)(b + 4) = 10000;
}
""")

# ---------------------------------------------------------------- window touch handling
w(0x4002F194, 148, "ui_windows_close_untouched",
  "From the window stack top index (0x42E68BC9) down to 0: stops at the first window whose rectangle contains the touch point (edges included); every window before it that is not self-contained (+2) is closed. ui_window_close gets the stock r1 (the +2 byte just read, so 0) and r2 (the last value compared).", TOUCH + WINH + """
void ui_window_close(u32 id, u32 r1, u32 r2);

void ui_windows_close_untouched(void)
{
    int i;
    for (i = WSTK[1]; i >= 0; i--) {
        u32 id = WSTK[2 + i];
        volatile unsigned char *p = WIN(id);
        int x = TCH16(4), y, a, r2;
        a = WH(p, 6);
        r2 = a;
        if (x >= a) {
            a = a + WH(p, 0xA);
            r2 = a;
            if (a >= x) {
                a = WH(p, 8);
                y = TCH16(6);
                r2 = a;
                if (y >= a) {
                    a = a + WH(p, 0xC);
                    r2 = a;
                    if (a >= y)
                        return;
                }
            }
        }
        if (p[2] == 0)
            ui_window_close(id, 0, (u32)r2);
    }
}
""")
w(0x4002F228, 148, "ui_window_touch_handle",
  "With a touch state (0x42E68BB0+0xC) pending: a touch strictly inside window id invalidates the windows from id (the stock r1 is the touch state) and clears the state; otherwise a release outside a window that is not self-contained runs ui_windows_close_untouched and clears the state.", TOUCH + WINH + """
void ui_window_invalidate_from(u32 id, u32 r1);
void ui_windows_close_untouched(void);

void ui_window_touch_handle(u32 id)
{
    u32 st = TCH8(0xC);
    volatile unsigned char *p;
    int x, y, a;
    if (st == 0)
        return;
    p = WIN(id);
    x = TCH16(4);
    a = WH(p, 6);
    if (a < x && x < a + WH(p, 0xA)) {
        a = WH(p, 8);
        y = TCH16(6);
        if (a < y && y < a + WH(p, 0xC)) {
            ui_window_invalidate_from(id, st);
            TCH8(0xC) = 0;
            return;
        }
    }
    if (st == 2 && p[2] == 0) {
        ui_windows_close_untouched();
        TCH8(0xC) = 0;
    }
}
""")
w(0x4002F0E8, 104, "draw_text_shadowed",
  "Draws s twice through 0x4002EE78: first at (x + dx, y + dy) (16 bit) in colour col << 24 (col and dy are the low bytes of the stack arguments), then at (x, y) in the colour word 0x42E68664 read on entry; 0x4002EA38 sets the colour.", """
void draw_colour_set(u32 c);
void func_4002ee78(u32 x, u32 y, u32 s);

void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col)
{
    u32 saved = *(volatile u32 *)0x42E68664u;
    draw_colour_set((col & 0xFF) << 24);
    func_4002ee78((x + dx) & 0xFFFF, (y + (dy & 0xFF)) & 0xFFFF, s);
    draw_colour_set(saved);
    func_4002ee78(x, y, s);
}
""")

# ---------------------------------------------------------------- per-screen group updaters
GROUPS = {
    0x4002F324: (84, [0x2, 0xC, 0x16, 0x20, 0x2A, 0x34, 0x3E, 0x48, 0x52]),
    0x4002F378: (76, [0x5C, 0x66, 0x70, 0x7A, 0x84, 0x8E, 0x98, 0xA2]),
    0x4002F3C4: (68, [0xAC, 0xB6, 0xC0, 0xCA, 0xD4, 0xDE, 0xE8]),
    0x4002F408: (8, [0xF2]),
    0x4002F410: (128, [0x17E, 0xFC, 0x192, 0x1A6, 0x1B0, 0x1BA, 0x1C4, 0x1CE, 0x1D8, 0x1E2, 0x1F6]),
    0x4002F490: (96, [0x20A, 0x214, 0x21E, 0x228, 0x17E, 0x232, 0x23C, 0x246]),
    0x4002F4F0: (252, [0x250, 0x25A, 0x264, 0x26E, 0x278, 0x282, 0x28C, 0x296, 0x2A0, 0x2AA, 0x2B4, 0x2BE, 0x2C8, 0x2D2, 0x2DC, 0x2E6, 0x2F0, 0x2FA, 0x304, 0x30E, 0x318, 0x322, 0x32C, 0x336]),
    0x4002F5EC: (40, [0x340, 0x34A, 0x354]),
    0x4002F614: (112, [0x35E, 0x368, 0x372, 0x37C, 0x386, 0x390, 0x39A, 0x3A4, 0x3AE, 0x3B8]),
    0x4002F684: (120, [0x3C2, 0x3CC, 0x3D6, 0x3E0, 0x3EA, 0x3F4, 0x3FE, 0x408, 0x412, 0x41C]),
    0x4002F6FC: (368, [0x426, 0x430, 0x43A, 0x444, 0x44E, 0x458, 0x462, 0x46C, 0x476, 0x480, 0x48A, 0x494, 0x49E, 0x4A8, 0x4B2, 0x4BC, 0x4C6, 0x4D0, 0x4DA, 0x4E4, 0x4EE, 0x4F8, 0x502, 0x50C, 0x516, 0x520, 0x52A, 0x534, 0x53E, 0x548, 0x552]),
    0x4002F86C: (116, [0x55C, 0x566, 0x570, 0x57A, 0x584, 0x58E, 0x598, 0x5A2, 0x5AC]),
    0x4002F8E0: (136, [0x5B6, 0x5C0, 0x5CA, 0x5D4, 0x5DE, 0x5E8, 0x5F2, 0x5FC, 0x606, 0x610, 0x61A]),
    0x4002FAA4: (8, [0x71E]),
    0x4002FB40: (8, [0x7FA]),
    0x4002FB48: (8, [0x804]),
    0x4002FB50: (72, [0x80E, 0x818, 0x822, 0x82C, 0x836]),
    0x4002FC30: (8, [0x912]),
    0x4002FCCC: (8, [0x9EE]),
    0x4002FCD4: (60, [0x9F8, 0xA02, 0xA0C, 0xA16]),
    0x4002FD10: (44, [0x106, 0x110, 0x11A]),
    0x4002FD3C: (60, [0x124, 0x12E, 0x138, 0x142, 0x14C]),
    0x4002FD78: (8, [0x188]),
    0x4002FD80: (52, [0x156, 0x160, 0x16A, 0x174]),
    0x4002FDB4: (8, [0x1EC]),
    0x4002FDBC: (8, [0x200]),
    0x4002FDC4: (8, [0x19C]),
    0x4002FE3C: (92, [0xAC0, 0xACA, 0xAD4, 0xADE, 0xAE8, 0xAF2, 0xAFC]),
    0x4002FE98: (80, [0xB06, 0xB10, 0xB1A, 0xB24, 0xB2E, 0xB38]),
    0x4002FEE8: (8, [0xB42]),
    0x4002FEF0: (8, [0xB4C]),
    0x4002FEF8: (80, [0xB56, 0xB60, 0xB6A, 0xB74, 0xB7E, 0xB88]),
    0x4002FF48: (92, [0xB92, 0xB9C, 0xBA6, 0xBB0, 0xBBA, 0xBC4, 0xBCE]),
    0x4002FFA4: (8, [0xBD8]),
    0x4002FFAC: (92, [0xBE2, 0xBEC, 0xBF6, 0xC00, 0xC0A, 0xC14, 0xC1E]),
    0x40030008: (8, [0xC28]),
}
for _a, (_size, _offs) in sorted(GROUPS.items()):
    _nm = ("touch_button_update_%04X" if len(_offs) == 1 else "touch_group_%04X") % _offs[0]
    _calls = "\n".join("    touch_button_update((volatile unsigned char *)0x%08Xu);" % (BTN + o) for o in _offs)
    w(_a, _size, _nm,
      "touch_button_update for the button record%s %s of the table 0x42E61FF8." % ("" if len(_offs) == 1 else "s", ", ".join("+0x%X" % o for o in _offs)),
      """void touch_button_update(volatile unsigned char *b);

void %s(void)
{
%s
}
""" % (_nm, _calls))

# ---------------------------------------------------------------- release handlers
REL = BTNH + """void ui_window_open(u32 id);
"""
w(0x40054F08, 68, "touch_btn_0804_release",
  "Updates button +0x804 (0x4002FB48); on its release (state 2) clears it and the table byte +0, then opens window 0x17.", REL + """
void touch_button_update_0804(void);

void touch_btn_0804_release(void)
{
    touch_button_update_0804();
    if (BTN8(0x804) != 2)
        return;
    BTN8(0x804) = 0;
    BTN8(0) = 0;
    ui_window_open(0x17);
}
""")
w(0x40055B98, 60, "touch_btn_01ec_release",
  "Updates button +0x1EC (0x4002FDB4); on its release clears it and the table byte +0, then runs the factory reset 0x400447C8 (it does not return).", REL + """
void touch_button_update_01EC(void);
void func_400447c8(void);

void touch_btn_01ec_release(void)
{
    touch_button_update_01EC();
    if (BTN8(0x1EC) != 2)
        return;
    BTN8(0x1EC) = 0;
    BTN8(0) = 0;
    func_400447c8();
}
""")
w(0x40055C78, 76, "touch_btn_019c_release",
  "Updates button +0x19C (0x4002FDC4); on its release clears it and the table byte +0, leaves USB mode, targets the back buffer, resets the windows, clears the frame and opens windows 1..5.", REL + """
void touch_button_update_019C(void);
void usb_mode_exit(void);
void fb_target_back(void);
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);

void touch_btn_019c_release(void)
{
    touch_button_update_019C();
    if (BTN8(0x19C) != 2)
        return;
    BTN8(0x19C) = 0;
    BTN8(0) = 0;
    usb_mode_exit();
    fb_target_back();
    ui_windows_reset();
    fb_clear_current();
    ui_windows_open_1_5();
}
""")

# ---------------------------------------------------------------- entry pad: more modes, sign
PAD = """#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))
"""
for _a, _m in ((0x4005FA78, 4), (0x4005FABC, 5), (0x4005FB00, 6)):
    w(_a, 68, "numpad_open_mode%d" % _m,
      "numpad_reset, mode byte +1 = %d, window 0x21 position = (x, y), then ui_window_open(0x21)." % _m, PAD + """void numpad_reset(void);
void ui_window_open(u32 id);

void numpad_open_mode%d(u32 x, u32 y)
{
    numpad_reset();
    PAD(1) = %d;
    WIN21[4] = y;
    WIN21[3] = x;
    ui_window_open(0x21);
}
""" % (_m, _m))
w(0x4005FC04, 128, "numpad_sign_apply",
  "Makes the text agree with the '-' flag +0x19: flag set and no leading '-': shift text +3..+0x15 right by one, put '-' at +3, length + 1; flag clear and a leading '-': shift +4..+0x16 left by one, length - 1.", PAD + """
void numpad_sign_apply(void)
{
    int i;
    int c = (signed char)PAD(3);
    if (PAD(0x19)) {
        if (c == '-')
            return;
        for (i = 0x15; i >= 3; i--)
            PAD(i + 1) = PAD(i);
        PAD(3) = '-';
        PAD(0x17) = PAD(0x17) + 1;
    } else {
        if (c != '-')
            return;
        for (i = 3; i < 0x16; i++)
            PAD(i) = PAD(i + 1);
        PAD(0x17) = PAD(0x17) - 1;
    }
}
""")

# ---------------------------------------------------------------- loose helpers
w(0x4005E10C, 32, "layer_clear", "fill_u16(0x400954A4 + n x 0x12BE70, 0, 0x95B50): clears layer n (1022 x 600 pixels of 16 bit).", """
void fill_u16(u32 dst, u32 v, u32 n);

void layer_clear(u32 n)
{
    fill_u16(0x400954A4u + n * 0x12BE70u, 0, 0x95B50);
}
""", src="display")
w(0x40056BE8, 24, "str_find_char", "Returns the first character of s that is NUL or equal to ch (the character, sign-extended, not a pointer).", """
int str_find_char(const signed char *s, int ch)
{
    for (;; s++) {
        int c = *s;
        if (c == 0 || c == ch)
            return c;
    }
}
""", src="string")
w(0x40056BE4, 4, "str_find_char_next", "str_find_char(s + 1, ch) (falls into 0x40056BE8).", """
int str_find_char(const signed char *s, int ch);

int str_find_char_next(const signed char *s, int ch)
{
    return str_find_char(s + 1, ch);
}
""", src="string")
w(0x4005D220, 92, "sine_table_lookup", "Table 0x42D564B4 of 20001 words: T[x mod 20001] for x >= 0, -T[(-x) mod 20001] for x < 0 (signed remainder, as the stock multiply-high division).", """
#define SINT ((volatile int *)0x42D564B4u)

int sine_table_lookup(int x)
{
    if (x < 0) {
        int n = (int)(0u - (u32)x);
        return -SINT[n % 20001];
    }
    return SINT[x % 20001];
}
""", src="display")
w(0x4005D27C, 96, "sine_table_build",
  "T[i] = (int)(sin(a_i) x 10000 x 10000 / 3.1415926) for i = 0..20000. Stock quirk: a_0 is the caller's d0 (taken as the parameter a0) and a_i = (i - 1) x 0.000314159265359 after that (the index is converted before it is incremented), so T[1] = sin(0) and each entry lags one step.", """
#define SINT ((volatile int *)0x42D564B4u)
double sin(double x);

void sine_table_build(double a0)
{
    int i;
    double a = a0;
    for (i = 0; i < 20001; i++) {
        double v = sin(a) * 10000.0;
        SINT[i] = (int)((v * 10000.0) / 3.1415926);
        a = (double)i * 0.000314159265359;
    }
}
""", src="display")
w(0x4006068C, 20, "func_400606a0_unless_locked", "Unless the byte 0x42E68BC7 is set, branches to 0x400606A0.", """
void func_400606a0(void);

void func_400606a0_unless_locked(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    func_400606a0();
}
""", src="init")
w(0x40062E00, 20, "func_40062e14_unless_locked", "Unless the byte 0x42E68BC7 is set, branches to 0x40062E14.", """
void measure_setup_62e14(void);

void func_40062e14_unless_locked(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    measure_setup_62e14();
}
""", src="init")
w(0x40060D68, 96, "calib_run_4_channels",
  "Block 0x42E68668: +0x190 = 1, +5 = 1; for i = 0..3: +0xF4 = i, then 0x400612FC, 0x40060FE8, 0x40060DC8, 0x40060BA4; then +0x190 = 0, +0xF4 = its old low byte, and settings_c_save.", """
#define CB8(o)  (*(volatile unsigned char *)(0x42E68668u + (o)))
#define CB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
void func_400612fc(void);
void func_40060fe8(void);
void func_40060dc8(void);
void func_40060ba4(void);
void settings_c_save(void);

void calib_run_4_channels(void)
{
    u32 old, i;
    CB8(0x190) = 1;
    old = CB32(0xF4);
    CB8(5) = 1;
    for (i = 0; i < 4; i++) {
        CB32(0xF4) = i;
        func_400612fc();
        func_40060fe8();
        func_40060dc8();
        func_40060ba4();
    }
    CB8(0x190) = 0;
    CB32(0xF4) = old & 0xFF;
    settings_c_save();
}
""", src="init")
w(0x4005C918, 60, "index_map_if_above",
  "When SYS+0x554 < d: simple_integer_index_mapping_nearest_neighbour(a, b, c, d, e, g) (e, g = the two stack words passed on).", """
void index_map_if_above(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g)
{
    if (SYS32(0x554) < d)
        simple_integer_index_mapping_nearest_neighbour(a, b, c, d, e, g);
}
""", src="acquisition")
w(0x40051C04, 36, "disk_format_and_reload", "disk_format, disk_mount, 0x4003486C, 0x400358B8, then 0x40036B00.", """
void disk_format(void);
void list_a_index_load(void);
void list_b_index_load(void);
void list_c_index_load(void);

void disk_format_and_reload(void)
{
    disk_format();
    disk_mount();
    list_a_index_load();
    list_b_index_load();
    list_c_index_load();
}
""", src="ui")
