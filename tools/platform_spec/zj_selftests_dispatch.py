# Two more generator self-tests, the touch-target test screen, the touch dispatcher of the window
# button handlers, and the generator settings writer to the FPGA.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wj(addr, size, name, desc, body, src="init"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wj(0x40050BAC, 552, "gen_selftest_50bac",
   "Self-test (DERIVED): range SYS+0x588 = 0x10, levels SYS+0x41 = SYS+0x105 = 0xB, SYS+0x54F = 1. For four output settings (channel 1 on with SYS+0x46 = 0 then 1; channel 2 on with SYS+0x10A = 0 then 1) it applies the outputs, runs one acquisition, then 30 acquisitions summing the spread (max - min, as a byte) of the capture bytes 0x42DF76D4[0x14 .. SYS+0x640 - 0x28). Pass when the sum with the setting = 1 is below 0.9f times the sum with 0, per channel: bytes 0x42E68B20 / 0x42E68B21, their AND in 0x42E68B1F.",
   """
void acq_defaults_reset(void);
void outputs_apply(void);
void acquisition_dispatcher(void);
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define CAP8(o) (*(volatile unsigned char *)(0x42DF76D4u + (o)))

static u32 spread_sum(int end)
{
    u32 s = 0, k;
    acquisition_dispatcher();
    for (k = 0; k < 30; k++) {
        u32 mx = 0, mn = 0xFF;
        int j;
        acquisition_dispatcher();
        for (j = 0x14; j < end; j++) {
            u32 b = CAP8(j);
            if (mx < b)
                mx = b;
            if (b < mn)
                mn = b;
        }
        s += (mx - mn) & 0xFF;
    }
    return s;
}

void gen_selftest_50bac(void)
{
    int end, a, b, c, d;
    u32 p1, p2;
    acq_defaults_reset();
    SYS8(0x54F) = 1;
    SYS8(0x588) = 0x10;
    SYS8(0x41) = 0xB;
    SYS8(0x105) = 0xB;
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS8(0x46) = 0;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    outputs_apply();
    end = (int)SYS16(0x640) - 0x28;
    a = (int)spread_sum(end);
    SYS8(0x46) = 1;
    outputs_apply();
    b = (int)spread_sum(end);
    SYS8(0x3E) = 0;
    SYS8(0x102) = 1;
    SYS8(0x10A) = 0;
    outputs_apply();
    c = (int)spread_sum(end);
    SYS8(0x10A) = 1;
    outputs_apply();
    d = (int)spread_sum(end);
    p1 = b < (int)((float)a * 0.9f);
    p2 = d < (int)((float)c * 0.9f);
    RS8(0x40) = p1;
    RS8(0x41) = p2;
    RS8(0x3F) = p2 & p1;
}
""")

wj(0x40050DDC, 504, "gen_selftest_50ddc",
   "Self-test (DERIVED): both channels on with SYS+0x46 = SYS+0x10A = 2, byte 0x42E68BC6 = 0; one generator setting (block 0x42E3F984: byte +0x2EFA = 0, words +0x2EFC = 0x2160EC0, +0x2F00 = 1000, +0x2F04 = 0; SYS+0x588 = 0xE, SYS+0x41 = SYS+0x105 = 6), applied, two acquisitions. Pass when 5 * (s64 SYS+0x910 / 1000) and 5 * (SYS+0x970 / 1000) are below 4 * 1000 (64-bit compare): bytes 0x42E68B23 / 0x42E68B24, their AND in 0x42E68B22.",
   """
void acq_defaults_reset(void);
void outputs_apply(void);
void gen_update(void);
void acquisition_dispatcher(void);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define S64(o) (*(volatile long long *)(0x40544E68u + (o)))

void gen_selftest_50ddc(void)
{
    int amp = 1000, q1, q2;
    u32 a, b;
    acq_defaults_reset();
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS8(0x46) = 2;
    SYS8(0x10A) = 2;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    GB8(0x2EFA) = 0;
    GB32(0x2EFC) = 0x2160EC0;
    GB32(0x2F00) = (u32)amp;
    GB32(0x2F04) = 0;
    SYS8(0x588) = 0xE;
    SYS8(0x41) = 6;
    SYS8(0x105) = 6;
    outputs_apply();
    gen_update();
    acquisition_dispatcher();
    acquisition_dispatcher();
    q1 = (int)(S64(0x970) / 1000);
    q2 = (int)(S64(0x910) / 1000);
    a = (long long)q2 * 5 < (long long)amp * 4;
    b = (long long)q1 * 5 < (long long)amp * 4;
    RS8(0x43) = a;
    RS8(0x44) = b;
    RS8(0x42) = b & a;
}
""")

wj(0x400519E4, 552, "touch_targets_test",
   "Touch test screen (DERIVED from what it draws): clears the touch state, draws five grey (0xCCCCCC) 80 x 80 squares at the four corners (20, 20), (924, 20), (20, 500), (924, 500) and the centre (472, 260), then polls the touch controller until each square has been touched (state byte 0x42E68BBC = 2 with x, y at 0x42E68BB4 / 0x42E68BB6 inside 21..99 of the square), redrawing a touched square in 0x88FF. Then clears the screen and byte 0x42E68B26.",
   """
void touch_state_clear(void);
void fb_target_back(void);
void fb_clear_current(void);
void draw_colour_set(u32 c);
void word_42e68c74_set(u32 v);
void half_42e68be2_set(u32 v);
void rect_fill(u32 x, u32 y, u32 w, u32 h);
void fb_present(void);
void gt911_poll(void);
#define TS8(o) (*(volatile unsigned char *)(0x42E68BB0u + (o)))
#define TS16(o) (*(volatile short *)(0x42E68BB0u + (o)))
#define IN(v, lo) ((u32)((v) - (lo)) < 0x4Fu)

void touch_targets_test(void)
{
    u32 got = 0;
    touch_state_clear();
    fb_target_back();
    fb_clear_current();
    draw_colour_set(0xCCCCCC);
    word_42e68c74_set(0x42E658D8);
    half_42e68be2_set(1);
    rect_fill(0x14, 0x14, 0x50, 0x50);
    rect_fill(0x39C, 0x14, 0x50, 0x50);
    rect_fill(0x14, 0x1F4, 0x50, 0x50);
    rect_fill(0x39C, 0x1F4, 0x50, 0x50);
    rect_fill(0x1D8, 0x104, 0x50, 0x50);
    fb_present();
    do {
        gt911_poll();
        if (TS8(0xC) == 2) {
            int x = TS16(4);
            u32 bit = 0, rx = 0, ry = 0;
            if (IN(x, 0x15) && IN(TS16(6), 0x15))
                bit = 1, rx = 0x14, ry = 0x14;
            else if (IN(x, 0x39D) && IN(TS16(6), 0x15))
                bit = 2, rx = 0x39C, ry = 0x14;
            else if (IN(x, 0x15) && IN(TS16(6), 0x1F5))
                bit = 4, rx = 0x14, ry = 0x1F4;
            else if (IN(x, 0x39D) && IN(TS16(6), 0x1F5))
                bit = 8, rx = 0x39C, ry = 0x1F4;
            else if (IN(x, 0x1D9) && IN(TS16(6), 0x105))
                bit = 0x10, rx = 0x1D8, ry = 0x104;
            if (bit) {
                got |= bit;
                draw_colour_set(0x88FF);
                rect_fill(rx, ry, 0x50, 0x50);
            }
        }
        fb_present();
    } while ((got & 0xFF) != 0x1F);
    fb_clear_current();
    *(volatile unsigned char *)(0x42E68AE0u + 0x46) = 0;
}
""")

DISPATCH = [
    (1, "main_menu_buttons"), (2, "buttons_05c_handler"), (3, "buttons_ac_open_windows"),
    (5, "touch_00f2_then_400545f8"), (6, "system_settings_buttons"), (7, "buttons_106_handler"),
    (8, "settings_slot_buttons"), (9, "language_select_buttons"), (10, "buttons_20a_handler"),
    (11, "func_400529f0"), (12, "file_lists_open_buttons"), (13, "channel1_settings_buttons"),
    (14, "channel2_settings_buttons"), (15, "func_400533a0"), (16, "output_select_buttons"),
    (17, "trigger_option_buttons"), (18, "func_40053b78"), (19, "button_71e_drag_handler"),
    (20, "list_a_thumbs_buttons"), (21, "list_a_delete_button_handler"), (22, "touch_btn_0804_release"),
    (23, "list_a_window_buttons"), (24, "list_b_window_buttons"), (25, "list_b_delete_button_handler"),
    (26, "list_c_window_buttons"), (27, "list_c_delete_button_handler"), (28, "buttons_9f8_handler"),
    (29, "system_calibration_button_handler"), (30, "touch_btn_01ec_release"), (31, "format_button_handler"),
    (32, "touch_btn_019c_release"), (33, "numpad_key_buttons"), (34, "buttons_ac0_handler"),
    (35, "numpad_buttons_b06"), (36, "cursor_drag_buttons_b42"), (37, "calibration_b4c_button_handler"),
    (38, "buttons_b56_handler"), (39, "measure_numpad_buttons"), (40, "marker_drag_buttons_bd8"),
    (41, "buttons_be2_handler"), (42, "calibration_button_handler"),
]

wj(0x40051E88, 520, "window_buttons_dispatch",
   "Touch dispatch of the windows: id 1..42 (no handler for 4) calls that window's button handler (main menu, settings, lists, numpads, calibration ...), then ui_window_touch_handle(id) for every id.",
   "".join("void %s(void);\n" % n for _, n in DISPATCH) + """void ui_window_touch_handle(u32 id);

void window_buttons_dispatch(u32 id)
{
    switch (id) {
""" + "".join("    case %d:\n        %s();\n        break;\n" % (i, n) for i, n in DISPATCH) + """    default:
        break;
    }
    ui_window_touch_handle(id);
}
""", src="ui")

wj(0x4004ED18, 504, "gen_fpga_settings_write",
   "Generator settings to the FPGA, skipped while byte 0x42E68BC7 is set. Block G = 0x42E3F984: clock = min(word 0x42E428E0, G+0x2EFC * point count G+0x2EF6); below 2500 it is multiplied by 10 (at least 2500) with the x10 flag G+0x2F0C = 1, else the flag is 0; stored in G+0x2F40 and sent to the MS5351 clock 1. Then FPGA selectors 0x31 / 0x32 = bits 8..15 and 0..7 of G+0x2F38 / G+0x2F3C, 0x3C = the x10 flag, 0x3D = 1, 0x41 = count - 2 (two bytes), 0x44 = (G+0x2EFA != 0); for each point i: 0x3F = i, 0x40 = sample halfword G+0xFB6 + 2i, 0x3E = 0 then 1 (strobe); then fpga_sel42_43_fill(low byte of G+0x2F46) and 0x3D = 0.",
   """
void ms5351_set_clk1(u32 hz);
void fpga_sel42_43_fill(u32 n);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB16(o) (*(volatile unsigned short *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))

void gen_fpga_settings_write(void)
{
    u32 f, c, i;
    volatile unsigned short *smp;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    f = GB16(0x2EF6) * GB32(0x2EFC);
    c = GB32(0x2F5C);
    if (c > f)
        c = f;
    if (c < 2500) {
        GB8(0x2F0C) = 1;
        c *= 10;
        if (c < 2500)
            c = 2500;
    } else {
        GB8(0x2F0C) = 0;
    }
    GB32(0x2F40) = c;
    ms5351_set_clk1(c);
    fpga_selector_command_write(0x31);
    fpga_data_byte_write((GB32(0x2F38) >> 8) & 0xFF);
    fpga_data_byte_write(GB32(0x2F38) & 0xFF);
    fpga_selector_command_write(0x32);
    fpga_data_byte_write((GB32(0x2F3C) >> 8) & 0xFF);
    fpga_data_byte_write(GB32(0x2F3C) & 0xFF);
    fpga_selector_command_write(0x3C);
    fpga_data_byte_write(GB8(0x2F0C));
    fpga_selector_command_write(0x3D);
    fpga_data_byte_write(1);
    fpga_selector_command_write(0x41);
    fpga_data_byte_write(((GB16(0x2EF6) - 2) >> 8) & 0xFF);
    fpga_data_byte_write((GB16(0x2EF6) - 2) & 0xFF);
    fpga_selector_command_write(0x44);
    fpga_data_byte_write(GB8(0x2EFA) != 0);
    smp = (volatile unsigned short *)(0x42E3F984u + 0xFB6);
    for (i = 0; i < GB16(0x2EF6); i++) {
        fpga_selector_command_write(0x3F);
        fpga_data_byte_write((i >> 8) & 0xFF);
        fpga_data_byte_write(i & 0xFF);
        fpga_selector_command_write(0x40);
        fpga_data_byte_write(*smp >> 8);
        fpga_data_byte_write(*smp & 0xFF);
        smp++;
        fpga_selector_command_write(0x3E);
        fpga_data_byte_write(0);
        fpga_selector_command_write(0x3E);
        fpga_data_byte_write(1);
    }
    fpga_sel42_43_fill(GB16(0x2F46) & 0xFF);
    fpga_selector_command_write(0x3D);
    fpga_data_byte_write(0);
}
""", src="acquisition")
