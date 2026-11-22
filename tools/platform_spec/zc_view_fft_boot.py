# View bounds and zoom fit, MMU section mapping, an FPGA fill sequence, FFT input and trace drawing,
# the storage check file, the factory step chain, window touch polling and the trace cursor values.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wc(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wc(0x40042280, 208, "view_bounds_update",
   "From the offsets word SYS+0x620 / +0x624 plus u16 SYS+0xC (64-bit sums): the start SYS+0x16 = clamp(a, 0, width SYS+8), the end = b - 1 clamped to 0 .. width - 1 (u16), SYS+0x18 = max(start, end) and SYS+0x1A = SYS+0x18 - start.", """
void view_bounds_update(void)
{
    long long c = SYS16(0xC), a = (long long)(int)SYS32(0x620) + c, b = (long long)(int)SYS32(0x624) + c;
    u32 lo, hi, m;
    if (a < 0) {
        lo = 0;
    } else {
        u32 w = SYS16(8);
        lo = a < (long long)w ? (u32)a : w;
    }
    SYS16(0x16) = lo;
    if (b < 1) {
        hi = 0;
    } else {
        u32 w = SYS16(8);
        hi = b < (long long)w - 1 ? 0xFFFF + (u32)b : 0xFFFF + w;
    }
    hi = (unsigned short)hi;
    m = SYS16(0x16);
    if (m < hi)
        m = hi;
    SYS16(0x18) = m;
    SYS16(0x1A) = m - SYS16(0x16);
}
""")

wc(0x400423B8, 164, "zoom_index_fit",
   "Zoom index byte SYS+0x598 (limited to below SYS+0x588): selects the entry (zoom_entry_select) and, while u64 SYS+0x590 / u64 SYS+0x5A0 / word SYS+0x5D4 is at least u16 SYS+0x2AC, steps the index up and invalidates window 3 (r1 = high word of the quotient). Finally the index is clamped to SYS+0x2A8 .. SYS+0x2A7.", """
void zoom_entry_select(void);
void ui_window_invalidate_from(u32 id, u32 r1);
#define Z64(o) (*(volatile unsigned long long *)(0x40544E68u + 0x590u + (o)))
#define Z8(o) (*(volatile unsigned char *)(0x40544E68u + 0x590u + (o)))

void zoom_index_fit(void)
{
    unsigned long long q;
    u32 k, lo, hi;
    if (Z8(8) >= SYS8(0x588))
        Z8(8) = SYS8(0x588) - 1;
    for (;;) {
        zoom_entry_select();
        q = Z64(0) / Z64(0x10);
        q = q / *(volatile u32 *)(0x40544E68u + 0x590u + 0x44);
        k = Z8(8);
        if ((u32)(q >> 32) == 0 && (u32)q < SYS16(0x2AC))
            break;
        Z8(8) = k + 1;
        ui_window_invalidate_from(3, (u32)(q >> 32));
    }
    lo = SYS8(0x2A8);
    if (k < lo)
        k = lo;
    hi = SYS8(0x2A7);
    if (hi > k)
        hi = k;
    Z8(8) = hi;
}
""")

wc(0x400429E4, 124, "mmu_map_sections",
   "Writes size >> 20 first-level section descriptors (1 MB each) into the table from entry va >> 20: physical section pa >> 20 upwards, flags 0xC12 (AP = 3, section) with C/B from attr bits 0..1.", """
void mmu_map_sections(volatile u32 *tbl, unsigned long long va, unsigned long long pa, unsigned long long size,
                      unsigned char attr)
{
    unsigned long long n = size >> 20;
    u32 s = (u32)(pa >> 20), cb = (attr & 3) << 2;
    volatile u32 *p = tbl + (u32)(va >> 20);
    while (n) {
        *p++ = 0xC12 | (s << 20) | cb;
        s++;
        n--;
    }
}
""", src="init")

wc(0x4004EC98, 128, "fpga_sel42_43_fill",
   "FPGA: selector 0x42 = 1, then 130 pairs (1, 0) to selector 0x43; selector 0x42 = 0, then n pairs (1, 0) to selector 0x43.", """
void fpga_selector_command_write(u32 sel);
void fpga_data_byte_write(u32 v);

void fpga_sel42_43_fill(u32 n)
{
    int i;
    fpga_selector_command_write(0x42);
    fpga_data_byte_write(1);
    fpga_selector_command_write(0x43);
    for (i = 0x82; i; i--) {
        fpga_data_byte_write(1);
        fpga_data_byte_write(0);
    }
    fpga_selector_command_write(0x42);
    fpga_data_byte_write(0);
    fpga_selector_command_write(0x43);
    for (; n; n--) {
        fpga_data_byte_write(1);
        fpga_data_byte_write(0);
    }
}
""", src="acquisition")

wc(0x4004FAB0, 132, "calibration_result_flags",
   "Clears the bytes 0x42E68B01..0x42E68B03, runs system_calibration and records its result: 1 sets all three to 1; 0 gives (+2, +3) = (0, 0), -1 gives (0, 1), -2 gives (1, 0); other results keep the values read back.", """
int system_calibration(void);
#define B8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))

void calibration_result_flags(void)
{
    int r;
    u32 a, b;
    B8(0x21) = 0;
    *(volatile unsigned short *)(0x42E68AE0u + 0x22) = 0;
    r = system_calibration();
    if (r == 1) {
        B8(0x21) = 1;
        B8(0x23) = 1;
        B8(0x22) = 1;
        return;
    }
    a = B8(0x22);
    b = B8(0x23);
    if (r == 0) {
        a = 0;
        b = 0;
    } else if (r == -1) {
        a = 0;
        b = 1;
    } else if (r == -2) {
        a = 1;
        b = 0;
    }
    B8(0x23) = b;
    B8(0x22) = a;
}
""")

wc(0x40041A4C, 340, "fft_input_prepare",
   "FFT input for the enabled channels (SYS+0x3E/+0x42, SYS+0x102/+0x106): n = word SYS+0x7F8 points taken every st = SYS+0x454 / n samples (at least 1) from the middle of the capture buffer (0x42AC7CF4, channel 2 at +0x62250), shifted right by SYS+0x264, as floats into 0x42E49EB4 / 0x42E4DEB4, followed by n zeros.", """
static u32 shr(u32 v, u32 s)
{
    s &= 0xFF;
    return s < 32 ? v >> s : 0;
}

void fft_input_prepare(void)
{
    u32 n = SYS32(0x7F8), q = n ? SYS32(0x454) / n : 0, off, i;
    int st = (int)q <= 0 ? 1 : (int)q;
    off = (SYS32(0x454) - st * n) >> 1;
    if (SYS8(0x3E) && SYS8(0x42)) {
        volatile unsigned short *s = (volatile unsigned short *)0x42AC7CF4u + off;
        volatile float *d = (volatile float *)0x42E49EB4u;
        u32 sh = SYS32(0x264);
        for (i = 0; i < n; i++, s += st)
            d[i] = (float)(int)shr(*s, sh);
        for (i = n; i < 2 * n; i++)
            d[i] = 0.0f;
    }
    if (SYS8(0x102) && SYS8(0x106)) {
        volatile unsigned short *s = (volatile unsigned short *)(0x42AC7CF4u + off * 2u + 0x62250u);
        volatile float *d = (volatile float *)0x42E4DEB4u;
        u32 sh = SYS32(0x264);
        for (i = 0; i < n; i++, s += st)
            d[i] = (float)(int)shr(*s, sh);
        for (i = n; i < 2 * n; i++)
            d[i] = 0.0f;
    }
}
""")

wc(0x40041E34, 288, "fft_traces_draw",
   "Draws the FFT traces of the enabled channels: for each x below width - 1 a vertical segment between the s16 values k and k + 1 (0x42E5B014.., channel 2 at 0x42E5C014..) plus the trace origin (y SYS+6; channel 2 adds half the height SYS+0xA), in 0xFF8800 and 0x88FF.", """
void draw_colour_set(u32 c);
void vline_draw(u32 x, u32 y0, u32 y1);

static void trace(volatile short *p, u32 x0, u32 y0)
{
    u32 i;
    for (i = 0; i < SYS16(8) - 1u; i++, p++) {
        int a = p[0], b = p[1];
        u32 x = (unsigned short)(i + x0);
        if (b < a)
            vline_draw(x, (unsigned short)(b + y0), (unsigned short)(a + y0));
        else
            vline_draw(x, (unsigned short)(a + y0), (unsigned short)(b + y0));
    }
}

void fft_traces_draw(void)
{
    if (SYS8(0x3E) && SYS8(0x42)) {
        u32 x0 = SYS16(4), y0 = SYS16(6);
        draw_colour_set(0xFF8800);
        trace((volatile short *)0x42E5B014u, x0, y0);
    }
    if (SYS8(0x102) && SYS8(0x106)) {
        u32 y0 = SYS16(6) + (SYS16(0xA) >> 1), x0 = SYS16(4);
        draw_colour_set(0x88FF);
        trace((volatile short *)0x42E5C014u, x0, y0);
    }
}
""", src="display")

wc(0x40051C28, 200, "disk_check_file",
   "Storage check of the POST: disk_format_and_reload, writes the 30-byte string at 0x4008DA20 into the file 0x4008E364 (`sd_nand_check_file.bin`), reads it back, deletes the file and sets byte 0x42E68AF0 to 1 when the two strings are equal (strcmp), 0 otherwise.", """
void __aeabi_memcpy(void *d, const void *s, u32 n);
void disk_format_and_reload(void);
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
int f_read(void *fp, void *buff, u32 btr, u32 *br);
int f_write(void *fp, const void *buff, u32 btw, u32 *bw);
int f_lseek(void *fp, unsigned long long ofs);
int f_unlink(const char *path);
int strcmp(const char *a, const char *b);

void disk_check_file(void)
{
    char rd[0x20], wr[0x20];
    unsigned char fil[0x250];
    const char *path = (const char *)0x4008E364u;
    __aeabi_memcpy(wr, (const void *)0x4008DA20u, 0x1E);
    disk_format_and_reload();
    f_open(fil, path, 0xB);
    f_lseek(fil, 0);
    f_write(fil, wr, 0x1E, 0);
    f_close(fil);
    f_open(fil, path, 3);
    f_lseek(fil, 0);
    f_read(fil, rd, 0x1E, 0);
    f_close(fil);
    f_unlink(path);
    *(volatile unsigned char *)0x42E68AF0u = strcmp(wr, rd) == 0;
}
""", src="init")

wc(0x40051CF0, 140, "factory_steps_run",
   "Unless the saved state word 0x42E68AE0 is 0x5A5555A5 (factory flow complete): runs each factory step whose done-word is not yet set (0x40050FE0 / +4 = 0xA5AA555A, 0x40051578 / +8 = 0x5A5AA5AA, 0x400519E4 / +0xC = 0xA5A5AA55, the last also setting the state word), saving after each (settings_e_save); then 0x400447C8, settings_load_all, state_blocks_init, outputs_apply and gen_update.", """
void factory_test_board(void);
void factory_test_tester(void);
void touch_targets_test(void);
void settings_e_save(void);
void factory_reset_keep_language(void);
void settings_load_all(void);
void state_blocks_init(void);
void outputs_apply(void);
void gen_update(void);
#define FS32(o) (*(volatile u32 *)(0x42E68AE0u + (o)))

void factory_steps_run(void)
{
    if (FS32(0) == 0x5A5555A5u)
        return;
    if (FS32(4) != 0xA5AA555Au) {
        factory_test_board();
        FS32(4) = 0xA5AA555Au;
        settings_e_save();
    }
    if (FS32(8) != 0x5A5AA5AAu) {
        factory_test_tester();
        FS32(8) = 0x5A5AA5AAu;
        settings_e_save();
    }
    if (FS32(0xC) != 0xA5A5AA55u) {
        touch_targets_test();
        FS32(0xC) = 0xA5A5AA55u;
        FS32(0) = 0x5A5555A5u;
        settings_e_save();
    }
    factory_reset_keep_language();
    settings_load_all();
    state_blocks_init();
    outputs_apply();
    gen_update();
}
""", src="init")

wc(0x40051E24, 100, "screenshot_button_poll",
   "For each open window (stack at 0x42E68BCA.., top index byte 0x42E68BC9) whose entry is 1, from the top down: updates touch button 0x42E62040 and, when it is released (state 2), clears it and byte 0x42E61FF8 and saves a screenshot (list_a_save_new).", """
void touch_button_update(volatile unsigned char *b);
void list_a_save_new(void);
#define WS8(o) (*(volatile unsigned char *)(0x42E68BC8u + (o)))

void screenshot_button_poll(void)
{
    int i;
    for (i = WS8(1); i >= 0; i--) {
        if (WS8(2 + i) == 1) {
            touch_button_update((volatile unsigned char *)0x42E62040u);
            if (*(volatile unsigned char *)0x42E62040u == 2) {
                *(volatile unsigned char *)0x42E62040u = 0;
                *(volatile unsigned char *)0x42E61FF8u = 0;
                list_a_save_new();
            }
        }
    }
}
""", src="ui")

wc(0x40052098, 104, "window_touch_poll",
   "Unless touch byte +0x17 is set: screenshot_button_poll, then the touch handler 0x40051E88 of each window of the stack from the top down, stopping after the first one whose byte +2 in its 14-byte record at 0x42E64520 is 0; finally touch byte +0xC = 0.", """
void screenshot_button_poll(void);
void window_buttons_dispatch(u32 id);
#define WS8(o) (*(volatile unsigned char *)(0x42E68BC8u + (o)))

void window_touch_poll(void)
{
    int i;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    screenshot_button_poll();
    for (i = WS8(1); i >= 0; i--) {
        u32 id = WS8(2 + i);
        window_buttons_dispatch(id);
        if (*(volatile unsigned char *)(0x42E64520u + id * 14 + 2) == 0)
            break;
    }
    *(volatile unsigned char *)(0x42E68BB0u + 0xC) = 0;
}
""", src="ui")

wc(0x400381E8, 388, "trace_cursor_values",
   "Unless touch byte +0x17 is set: converts the two cursor positions (words 0x42E22D2C/30) to values. Horizontally: 10^(base + pos / width * scale) for both, their difference; vertically, from the per-x tables at 0x42E20DE8 (gain) / 0x42E21D88 (phase), the two levels in each of two scales and their differences. Results are doubles at 0x42E22D38..0x42E22D78 (block 0x42E180C8 + 0xAC70..0xACB0).", """
double pow(double x, double y);
#define TD(o) (*(volatile double *)(0x42E180C8u + (o)))
#define TI(o) (*(volatile int *)(0x42E180C8u + (o)))
#define TH(o) (*(volatile unsigned short *)(0x42E180C8u + (o)))

void trace_cursor_values(void)
{
    double sc, w, base, p1, p2, top, bot, h, s5, a1, a2, r, b1, b2;
    int c1, c2;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    sc = TD(0x8D18);
    w = (double)TH(4);
    base = TD(0x8D08);
    p1 = pow(10.0, base + (double)TI(0xAC64) / w * sc);
    p2 = pow(10.0, base + (double)TI(0xAC68) / w * sc);
    top = TD(0x10);
    c1 = TI(0xAC64);
    bot = TD(0x18);
    h = (double)TH(6);
    c2 = TI(0xAC68);
    TD(0xAC70) = p1;
    TD(0xAC78) = p2;
    TD(0xAC80) = __builtin_fabs(p2 - p1);
    r = top - bot;
    a1 = top - r * (double)TH(0x8D20 + 2 * c1) / h;
    a2 = top - r * (double)TH(0x8D20 + 2 * c2) / h;
    s5 = TD(0x20) - TD(0x28);
    b2 = s5 * (double)TH(0x9CC0 + 2 * c2);
    b1 = s5 * (double)TH(0x9CC0 + 2 * c1);
    TD(0xAC88) = a1;
    TD(0xAC90) = a2;
    b2 = b2 / h;
    b1 = b1 / h;
    TD(0xACA8) = TD(0x20) - b2;
    TD(0xAC98) = __builtin_fabs(a2 - a1);
    TD(0xACA0) = TD(0x20) - b1;
    TD(0xACB0) = __builtin_fabs(TD(0xACA8) - TD(0xACA0));
}
""")
