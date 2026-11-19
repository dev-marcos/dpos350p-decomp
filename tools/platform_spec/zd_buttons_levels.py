# Touch-button handlers of the file, language, format and calibration windows, a cursor clamp, the FPGA
# counter sequences of selectors 0x96.., level statistics, the second FFT input and a measurement setup.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wd(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


# ui_window_close(id) is declared here with one argument: the stock callers leave in r1/r2 whatever the
# preceding call returned there (ui_window_close only uses r2 when the window is not on the stack).
TBH = '''#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
'''

wd(0x40055344, 76, "list_b_delete_button_handler",
   "Button 0x912 released (state 2): clears it and the touch byte, deletes the marked entries of list B, closes window 0x19 and redraws the thumbnails.", TBH + """
void touch_button_update_0912(void);
void list_b_delete_marked(void);
void list_b_page_thumbs(void);

void list_b_delete_button_handler(void)
{
    touch_button_update_0912();
    if (TB(0x912) != 2)
        return;
    TB(0x912) = 0;
    TB(0) = 0;
    list_b_delete_marked();
    ui_window_close(0x19);
    list_b_page_thumbs();
}
""")

wd(0x40054EB8, 80, "list_a_delete_button_handler",
   "Button 0x7FA released: clears it and the touch byte, deletes the marked entries of list A, closes window 0x15, redraws the thumbnails and reloads the current entry.", TBH + """
void touch_button_update_07FA(void);
void list_a_delete_marked(void);
void list_a_page_thumbs(void);
void list_a_current_load(void);

void list_a_delete_button_handler(void)
{
    touch_button_update_07FA();
    if (TB(0x7FA) != 2)
        return;
    TB(0x7FA) = 0;
    TB(0) = 0;
    list_a_delete_marked();
    ui_window_close(0x15);
    list_a_page_thumbs();
    list_a_current_load();
}
""")

wd(0x400555E8, 72, "list_c_delete_button_handler",
   "Button 0x9EE released: clears it and the touch byte, deletes the marked records of list C and closes window 0x1B.", TBH + """
void touch_button_update_09EE(void);
void list_c_delete_marked(void);

void list_c_delete_button_handler(void)
{
    touch_button_update_09EE();
    if (TB(0x9EE) != 2)
        return;
    TB(0x9EE) = 0;
    TB(0) = 0;
    list_c_delete_marked();
    ui_window_close(0x1B);
}
""")

wd(0x40055720, 120, "buttons_106_handler",
   "Buttons 0x106, 0x110, 0x11A: the first one released (state 2) is cleared with the touch byte and window 8 is opened.", TBH + """
void touch_group_0106(void);

void buttons_106_handler(void)
{
    touch_group_0106();
    if (TB(0x106) == 2) {
        TB(0x106) = 0;
    } else if (TB(0x110) == 2) {
        TB(0x110) = 0;
    } else if (TB(0x11A) == 2) {
        TB(0x11A) = 0;
    } else {
        return;
    }
    TB(0) = 0;
    ui_window_open(8);
}
""")

wd(0x40035540, 56, "list_a_reload",
   "Reloads list A: index, marks cleared, SYS+0xF90 = 0 with the u16 SYS+0xFA4 and SYS+0xF9E, cursor clamp, page check; then falls through into list_a_page_thumbs.", """
void list_a_index_load(void);
void list_a_marks_clear(void);
void list_cursor_clamp_40035248(void);
void list_a_page_validate(void);
void list_a_page_thumbs(void);

void list_a_reload(void)
{
    list_a_index_load();
    list_a_marks_clear();
    SYS8(0xF90) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
    list_cursor_clamp_40035248();
    list_a_page_validate();
    list_a_page_thumbs();
}
""")

wd(0x40052F08, 172, "file_lists_open_buttons",
   "Buttons 0x340, 0x34A, 0x354: the first one released resets the window stack and opens window 0x14 (list A, list_a_reload), 0x18 (reload_chain_40035744) or 0x1A (reload_chain_400378e0).", TBH + """
void touch_group_0340(void);
void ui_windows_reset(void);
void list_a_reload(void);
void reload_chain_40035744(void);
void reload_chain_400378e0(void);

void file_lists_open_buttons(void)
{
    touch_group_0340();
    if (TB(0x340) == 2) {
        TB(0x340) = 0;
        TB(0) = 0;
        ui_windows_reset();
        ui_window_open(0x14);
        list_a_reload();
    } else if (TB(0x34A) == 2) {
        TB(0x34A) = 0;
        TB(0) = 0;
        ui_windows_reset();
        ui_window_open(0x18);
        reload_chain_40035744();
    } else if (TB(0x354) == 2) {
        TB(0x354) = 0;
        TB(0) = 0;
        ui_windows_reset();
        ui_window_open(0x1A);
        reload_chain_400378e0();
    }
}
""")

wd(0x40055AD4, 196, "language_select_buttons",
   "Buttons 0x156, 0x160, 0x16A, 0x174 set the language byte SYS+0x7AC to 1..4 (the last one leaves the touch byte set), then apply the texts, save settings block B and mark the windows dirty.", TBH + """
void touch_group_0156(void);
void ui_text_language_apply(void);
void settings_b_save(void);
void ui_windows_mark_dirty(void);

void language_select_buttons(void)
{
    touch_group_0156();
    if (TB(0x156) == 2) {
        TB(0x156) = 0;
        TB(0) = 0;
        SYS8(0x7AC) = 1;
    } else if (TB(0x160) == 2) {
        TB(0x160) = 0;
        TB(0) = 0;
        SYS8(0x7AC) = 2;
    } else if (TB(0x16A) == 2) {
        TB(0x16A) = 0;
        TB(0) = 0;
        SYS8(0x7AC) = 3;
    } else if (TB(0x174) == 2) {
        TB(0x174) = 0;
        SYS8(0x7AC) = 4;
    } else {
        return;
    }
    ui_text_language_apply();
    settings_b_save();
    ui_windows_mark_dirty();
}
""")

wd(0x40055634, 228, "buttons_9f8_handler",
   "Buttons 0x9F8 and 0xA02 (the first also runs reload_chain_400373e0) leave mode 0x3C and swap windows 0x1C/5 for 2/5; buttons 0xA0C and 0xA16 set byte 0x4091F398 to 0 or 1 and invalidate from window 5.", TBH + """
void touch_group_09F8(void);
void reload_chain_400373e0(void);
void sys_3c_leave(void);
void ui_window_invalidate_from(u32 id, u32 r1);
#define FLAG (*(volatile unsigned char *)0x4091F398u)

void buttons_9f8_handler(void)
{
    touch_group_09F8();
    if (TB(0x9F8) == 2) {
        TB(0x9F8) = 0;
        TB(0) = 0;
        reload_chain_400373e0();
    } else if (TB(0xA02) == 2) {
        TB(0xA02) = 0;
        TB(0) = 0;
    } else {
        u32 r1;                      /* the stock leaves r1 = flag address or flag value */
        if (TB(0xA0C) == 2) {
            TB(0xA0C) = 0;
            TB(0) = 0;
            FLAG = 0;
            r1 = 0x4091F398u;
        } else if (TB(0xA16) == 2) {
            TB(0xA16) = 0;
            TB(0) = 0;
            FLAG = 1;
            r1 = 1;
        } else {
            return;
        }
        ui_window_invalidate_from(5, r1);
        return;
    }
    sys_3c_leave();
    ui_window_close(0x1C);
    ui_window_close(5);
    ui_window_open(2);
    ui_window_open(5);
}
""")

MSG = '''void message_box_draw(int x, int y, u32 text, int align);
void delay_ms(u32 ms);
#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define WR16(o) (*(volatile unsigned short *)(0x42E64520u + (o)))
'''

wd(0x40055BD4, 164, "format_button_handler",
   "Button 0x200 released: formats the disk (disk_format), shows the message of text entry 0xAAA0 at the centre of window rectangle 0x1B8 (plus its offsets), waits 500 ms, restores the area and closes window 0x1F.", TBH + MSG + """
void touch_button_update_0200(void);
int disk_format(void);
void rect_42e68c38_draw(void);

void format_button_handler(void)
{
    u32 cx, cy;
    touch_button_update_0200();
    cy = WR16(0x1BA) + (WR16(0x1BE) >> 1);
    cx = WR16(0x1B8) + (WR16(0x1BC) >> 1);
    if (TB(0x200) != 2)
        return;
    TB(0x200) = 0;
    TB(0) = 0;
    disk_format();
    message_box_draw(TX8(0xAAA0 + 0x194) + (int)(cx & 0xFFFF), TX8(0xAAA0 + 0x195) + (int)(cy & 0xFFFF),
                     TX32(0xAAA0), 0);
    delay_ms(500);
    rect_42e68c38_draw();
    ui_window_close(0x1F);
}
""")

wd(0x40056974, 208, "calibration_button_handler",
   "Button 0xC28 released: shows message 0x9C00 at the centre of window rectangle 0x252, runs calib_run_4_channels, shows message 0x9C00 + 0x1A0, waits 700 ms and closes window 0x2A.", TBH + MSG + """
void touch_button_update_0C28(void);
void calib_run_4_channels(void);

void calibration_button_handler(void)
{
    u32 cx, cy;
    touch_button_update_0C28();
    cx = (unsigned short)(WR16(0x252) + (WR16(0x256) >> 1));
    cy = (unsigned short)(WR16(0x254) + (WR16(0x258) >> 1));
    if (TB(0xC28) != 2)
        return;
    TB(0xC28) = 0;
    TB(0) = 0;
    message_box_draw((int)cx + TX8(0x9C00 + 0x194), (int)cy + TX8(0x9C00 + 0x195), TX32(0x9C00), 0);
    calib_run_4_channels();
    message_box_draw((int)cx + TX8(0x9C00 + 0x334), (int)cy + TX8(0x9C00 + 0x335), TX32(0x9C00 + 0x1A0), 0);
    delay_ms(700);
    ui_window_close(0x2A);
}
""")

wd(0x40054540, 168, "button_71e_drag_handler",
   "Button 0x71E pressed (state 1): clears it and the touch byte, touch_moved_flag and, in drag mode 0x12 (byte 0x42E68998), keeps s16 SYS+0x27E as the 64-bit start. Else when byte 0x71F is set: clears it, sets the touch byte, touch_drag_delta_b and in mode 0x12 sets SYS+0x27E = start + delta (word +0x34).", TBH + """
void touch_button_update_071E(void);
void touch_moved_flag(void);
void touch_drag_delta_b(void);
#define DG8(o) (*(volatile unsigned char *)(0x42E68998u + (o)))
#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))
#define DG64(o) (*(volatile long long *)(0x42E68998u + (o)))

void button_71e_drag_handler(void)
{
    touch_button_update_071E();
    if (TB(0x71E) == 1) {
        TB(0x71E) = 0;
        TB(0) = 0;
        touch_moved_flag();
        if (DG8(0) == 0x12)
            DG64(8) = (short)SYS16(0x27E);
    } else if (TB(0x71F)) {
        TB(0x71F) = 0;
        TB(0) = 1;
        touch_drag_delta_b();
        if (DG8(0) == 0x12)
            SYS16(0x27E) = (unsigned short)(DG32(0x34) + DG32(8));
    }
}
""")

wd(0x40063484, 108, "cursor_27e_clamp",
   "Clamps the s16 SYS+0x27E to m .. height - m (m = u16 SYS+0x1AE, height SYS+0xA), or to 2m .. height - 2m when byte SYS+0x2AA is set; the upper limit is tested first.", """
void cursor_27e_clamp(void)
{
    int m = SYS16(0x1AE), h = SYS16(0xA), v = (short)SYS16(0x27E);
    if (SYS8(0x2AA) == 0) {
        if (h - m < v)
            v = h - m;
        else if (v < m)
            v = m;
    } else {
        if (h - 2 * m < v)
            v = h - 2 * m;
        else if (v < 2 * m)
            v = 2 * m;
    }
    SYS16(0x27E) = v;
}
""", src="trace")

FPG = '''void fpga_selector_command_write(u32 sel);
void fpga_data_byte_write(u32 v);
'''

wd(0x40063BC0, 116, "fpga_counters_init",
   "For the 8 counters i: clears SYS+0x674+i and the last value SYS+0xBC0+i, selector 0x96 + 2i = 1, then 130 pairs (1, 0) to selector 0x97 + 2i.", FPG + """
void fpga_counters_init(void)
{
    int i, k;
    for (i = 0; i < 8; i++) {
        SYS8(0x674 + i) = 0;
        SYS8(0xBC0 + i) = 0;
        fpga_selector_command_write((0x96 + 2 * i) & 0xFF);
        fpga_data_byte_write(1);
        fpga_selector_command_write((0x97 + 2 * i) & 0xFF);
        for (k = 0x82; k; k--) {
            fpga_data_byte_write(1);
            fpga_data_byte_write(0);
        }
    }
}
""", src="acquisition")

wd(0x40063C34, 124, "fpga_counter_step",
   "Counter ch: when the target SYS+0x67C+ch differs from the last value SYS+0xBC0+ch, writes the direction (1 = down) to selector 0x96 + 2ch and |difference| pairs (1, 0) to selector 0x97 + 2ch, then keeps the target as the last value.", FPG + """
void fpga_counter_step(u32 ch)
{
    int d;
    u32 cur = SYS8(0x67C + ch), old = SYS8(0xBC0 + ch);
    if (cur == old)
        return;
    d = (int)cur - (int)old;
    fpga_selector_command_write((2 * ch + 0x96) & 0xFF);
    if (d < 0) {
        d = -d;
        fpga_data_byte_write(1);
    } else {
        fpga_data_byte_write(0);
    }
    fpga_selector_command_write((2 * ch + 0x97) & 0xFF);
    for (; d > 0; d--) {
        fpga_data_byte_write(1);
        fpga_data_byte_write(0);
    }
    SYS8(0xBC0 + ch) = SYS8(0x67C + ch);
}
""", src="acquisition")

LV = '''#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile int *)(0x42E68668u + (o)))
#define SRC (*(volatile short *volatile *)0x42E68CB0u)

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}
'''

wd(0x400621EC, 144, "level_stats_5f010",
   "For n = word +0xB8 samples of the s16 source (pointer 0x42E68CB0): v clamped to 1 .. top - 1 (top = word +0xC0), stores top - v as u16 at 0x42E5F010 and keeps the mean (+0x160), max (+0x164) and min (+0x168) of block 0x42E68668.", LV + """
void level_stats_5f010(void)
{
    int n = MB32(0xB8), top = MB32(0xC0), mx = -32767, mn = 32767, i, v;
    u32 sum = 0;
    volatile short *s = SRC;
    volatile unsigned short *d = (volatile unsigned short *)0x42E5F010u;
    for (i = 0; i < n; i++) {
        v = *s++;
        if (top - 1 < v)
            v = top - 1;
        else if (v <= 0)
            v = 1;
        v = top - v;
        *d++ = v;
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += v;
    }
    MB32(0x160) = sdv((int)sum, n);
    MB32(0x164) = mx;
    MB32(0x168) = mn;
}
""", src="trace")

wd(0x40062284, 164, "level_stats_60008",
   "When byte +1 of block 0x42E68668 is set: for n = word +0xDC samples, v = sample * scale (+0xE0) / word +0xC0, clamped to 0 .. scale, stores scale - v as u16 at 0x42E60008 and keeps the mean (+0x178), max (+0x17C) and min (+0x180).", LV + """
void level_stats_60008(void)
{
    int n, sc, dv, mx = -32767, mn = 32767, i, v;
    u32 sum = 0;
    volatile short *s;
    volatile unsigned short *d = (volatile unsigned short *)0x42E60008u;
    if (MB8(1) == 0)
        return;
    sc = MB32(0xE0);
    n = MB32(0xDC);
    s = SRC;
    dv = MB32(0xC0);
    for (i = 0; i < n; i++) {
        v = sdv((int)((u32)sc * (u32)(int)*s++), dv);
        if (sc < v)
            v = sc;
        else if (v < 0)
            v = 0;
        v = sc - v;
        *d++ = v;
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += v;
    }
    MB32(0x178) = sdv((int)sum, n);
    MB32(0x17C) = mx;
    MB32(0x180) = mn;
}
""", src="trace")

wd(0x40061580, 132, "fft_input_prepare_b",
   "n = word +0xF8 of block 0x42E68668 consecutive samples from the middle of the capture buffer 0x42AC7CF4, shifted right by SYS+0x264, as floats into 0x42D164B4, followed by n zeros (when n < 2n as u32).", """
static u32 shr(u32 v, u32 s)
{
    s &= 0xFF;
    return s < 32 ? v >> s : 0;
}

void fft_input_prepare_b(void)
{
    u32 n = *(volatile u32 *)(0x42E68668u + 0xF8), i;
    volatile float *d = (volatile float *)0x42D164B4u;
    if (n) {
        volatile unsigned short *s = (volatile unsigned short *)0x42AC7CF4u + ((SYS32(0x454) - n) >> 1);
        u32 sh = SYS32(0x264);
        for (i = 0; i < n; i++)
            d[i] = (float)(int)shr(s[i], sh);
    }
    if (n < (u32)(n << 1)) {
        for (i = 0; i < n; i++)
            d[n + i] = 0.0f;
    }
}
""", src="acquisition")

wd(0x40063798, 148, "fpga_position_290_update",
   "Reads FPGA flag 0x18 (fpga_cmd18_read). Flag set: word SYS+0x290 = (13-bit value of selector 0x14) - SYS+0x28C, plus SYS+0x520 when negative. Else: 0 when u64 SYS+0x30 < SYS+0x28C, otherwise (u64 SYS+0x30 - SYS+0x28C) mod SYS+0x520 (the remainder of __aeabi_uldivmod).", """
void fpga_cmd18_read(void);
void fpga_selector_command_write(u32 sel);

void fpga_position_290_update(void)
{
    u32 r;
    fpga_cmd18_read();
    if (SYS8(0x279)) {
        u32 a, v;
        fpga_selector_command_write(0x14);
        a = fpga_data_byte_read();
        v = fpga_data_byte_read() | (a << 8);
        r = (v & 0x1FFF) - SYS32(0x28C);
        if ((int)r < 0)
            r += SYS32(0x520);
    } else {
        unsigned long long t = *(volatile unsigned long long *)(0x40544E68u + 0x30);
        u32 x = SYS32(0x28C);
        if (t < x)
            r = 0;
        else
            r = (u32)((t - x) % SYS32(0x520));
    }
    SYS32(0x290) = r;
}
""", src="acquisition")

wd(0x40062E14, 200, "measure_setup_62e14",
   "Block 0x42E68668: byte +0 = 0, runs 0x400612FC and 0x40060FE8, byte +5 = 1, runs 0x40060DC8 and 0x40061F10; then d = (+0x10 - (+0x20 + +0x28) / 2) / 6 into +0x140 and +0x10 + d / 2 into +0x148; v = (s64)+0x48: +0x138 = 2v, +0x130 = v, +0x120 = v - 2v / 2, +0x128 = v + 2v / 2.", """
void func_400612fc(void);
void auto_level_search(void);
void fft_spur_cancel(void);
void func_40061f10(void);
#define MD(o) (*(volatile double *)(0x42E68668u + (o)))
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

void measure_setup_62e14(void)
{
    double d;
    long long v, t;
    *(volatile unsigned char *)0x42E68668u = 0;
    func_400612fc();
    auto_level_search();
    *(volatile unsigned char *)(0x42E68668u + 5) = 1;
    fft_spur_cancel();
    func_40061f10();
    d = (MD(0x10) - (MD(0x20) + MD(0x28)) * 0.5) / 6.0;
    MD(0x140) = d;
    MD(0x148) = MD(0x10) + d * 0.5;
    v = (long long)MD(0x48);
    t = (long long)((unsigned long long)v * 2u);
    M64(0x138) = t;
    M64(0x130) = v;
    M64(0x120) = (long long)((unsigned long long)v - (unsigned long long)(t / 2));
    M64(0x128) = (long long)((unsigned long long)v + (unsigned long long)(M64(0x138) / 2));
}
""", src="acquisition")
