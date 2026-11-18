# Small leftovers: global setters, gates on touch byte +0x17 and on SYS+0x652, one-byte FPGA
# commands from SYS fields, drawing wrappers (pixel, colour, spans, rectangles), the display
# start-up chain and a few fixed call sequences.
# exec'd by gen_platform.py


def w(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


TOUCH = """#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */
"""

# ---------------------------------------------------------------- setters
w(0x4002CCCC, 12, "word_42e68c74_set", "*(u32 *)0x42E68C74 = a.", """
void word_42e68c74_set(u32 a)
{
    *(volatile u32 *)0x42E68C74u = a;
}
""")
w(0x4002DC40, 12, "half_42e68be2_set", "*(u16 *)0x42E68BE2 = a.", """
void half_42e68be2_set(u32 a)
{
    *(volatile unsigned short *)0x42E68BE2u = a;
}
""")
w(0x4002DC74, 12, "byte_42e68be0_set", "*(u8 *)0x42E68BE0 = a.", """
void byte_42e68be0_set(u32 a)
{
    *(volatile unsigned char *)0x42E68BE0u = a;
}
""")
w(0x400342CC, 16, "draw_target_default",
  "The drawing target pointer 0x40095498 = 0x41FB8FE8 (pixel_put writes through this pointer).", """
void draw_target_default(void)
{
    *(volatile u32 *)0x40095498u = 0x41FB8FE8u;
}
""", src="display")

# ---------------------------------------------------------------- gates
for a, t in ((0x40023E90, 0x40023EA4), (0x40024C4C, 0x40024C60), (0x40028350, 0x40028364),
             (0x40037C28, 0x40037C3C)):
    w(a, 20, "func_%08x_unless_t17" % t, "Unless touch byte 0x42E68BB0+0x17 is set, branches to 0x%08X." % t, TOUCH + """
void func_%08x(void);

void func_%08x_unless_t17(void)
{
    if (TCH8(0x17) == 0)
        func_%08x();
}
""" % (t, t, t))
w(0x40024E14, 20, "func_40023ea4_or_40024c60",
  "SYS+0x652 == 0: the gate of 0x40023EA4, else the gate of 0x40024C60.", """
void func_40023ea4_unless_t17(void);
void func_40024c60_unless_t17(void);

void func_40023ea4_or_40024c60(void)
{
    if (SYS8(0x652) == 0)
        func_40023ea4_unless_t17();
    else
        func_40024c60_unless_t17();
}
""")
w(0x4002CC54, 20, "func_4002c7ac_or_4002c5f8", "SYS+0x652 == 0: 0x4002C7AC, else 0x4002C5F8.", """
void func_4002c7ac(void);
void func_4002c5f8(void);

void func_4002c7ac_or_4002c5f8(void)
{
    if (SYS8(0x652) == 0)
        func_4002c7ac();
    else
        func_4002c5f8();
}
""")
w(0x4002CC6C, 48, "sys_rect_redraw_unless_t17",
  "Unless touch byte +0x17 is set: traces_e10c_enabled, sys_rect_fill, then func_4002c7ac_or_4002c5f8.", TOUCH + """
u32 traces_e10c_enabled(void);
void sys_rect_fill(void);
void func_4002c7ac_or_4002c5f8(void);

void sys_rect_redraw_unless_t17(void)
{
    if (TCH8(0x17))
        return;
    traces_e10c_enabled();
    sys_rect_fill();
    func_4002c7ac_or_4002c5f8();
}
""")

# ---------------------------------------------------------------- drawing wrappers
w(0x4002EB70, 56, "pixel_put",
  "Pixel (x, y): when byte 0x42E6865C is set, 0x4002EA88(x, y) (the blended path); else the u16 at 0x42E68660 is stored at target + 2 * (x + 1022 * y), target = *(u32 *)0x40095498.", """
void pixel_blend(u32 x, u32 y);

void pixel_put(u32 x, u32 y)
{
    if (*(volatile unsigned char *)0x42E6865Cu) {
        pixel_blend(x, y);
        return;
    }
    *(volatile unsigned short *)(*(volatile u32 *)0x40095498u + (x + y * 1022u) * 2u) =
        *(volatile unsigned short *)0x42E68660u;
}
""", src="display")
w(0x4002EBA8, 32, "func_4002ecc4_span", "(a, b, c): 0x4002ECC4(b, a, (u16)(c - b + 1), 1).", """
void rect_fill(u32 a, u32 b, u32 c, u32 d);

void func_4002ecc4_span(u32 a, u32 b, u32 c)
{
    rect_fill(b, a, (c - b + 1) & 0xFFFF, 1);
}
""", src="display")
w(0x4002C5C4, 48, "sys_rect_fill", "Colour 0 (0x4002EA38), then 0x4002ECC4 with the four u16 at SYS+4, +6, +8, +0xA.", """
void draw_colour_set(u32 c);
void rect_fill(u32 a, u32 b, u32 c, u32 d);

void sys_rect_fill(void)
{
    draw_colour_set(0);
    rect_fill(SYS16(4), SYS16(6), SYS16(8), SYS16(0xA));
}
""", src="display")
w(0x40034284, 48, "rect_42e68c38_draw",
  "u16 x, y, w, h at 0x42E68C38: 0x4003436C(x, y, (u16)(x + w + 1), (u16)(y + h + 1)).", """
void fb_rect_copy_from_e4e84(u32 a, u32 b, u32 c, u32 d);

void rect_42e68c38_draw(void)
{
    volatile unsigned short *h = (volatile unsigned short *)0x42E68C38u;
    u32 x = h[0], y = h[1];
    fb_rect_copy_from_e4e84(x, y, (h[2] + x + 1) & 0xFFFF, (h[3] + y + 1) & 0xFFFF);
}
""", src="display")
w(0x40031710, 48, "colour_toggle_42e68d1a",
  "Toggles byte 0x42E68D1A and sets the colour 0x10262626 (was set) or 0x10303030 (was clear) through 0x4002EA38.", """
void draw_colour_set(u32 c);

void colour_toggle_42e68d1a(void)
{
    volatile unsigned char *p = (volatile unsigned char *)0x42E68D1Au;
    if (*p) {
        *p = 0;
        draw_colour_set(0x10262626u);
    } else {
        *p = 1;
        draw_colour_set(0x10303030u);
    }
}
""", src="display")
w(0x4001C348, 52, "ui_window_05_draw",
  "Draw function of window 5: touch_button_set_rect(rectangle of window record 5 = u16 at 0x42E64520+0x4C..+0x52, button record 0x42E620EA), then 0x400346C0.", """
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);
void ui_frame_chain(void);

void ui_window_05_draw(void)
{
    volatile unsigned short *wn = (volatile unsigned short *)0x42E64520u;
    touch_button_set_rect(wn[0x26], wn[0x27], wn[0x28], wn[0x29], (volatile unsigned char *)0x42E620EAu);
    ui_frame_chain();
}
""")
w(0x4002DC4C, 40, "draw_state_defaults",
  "table_scatter_42cf64b4, 0x4002CCD8, byte_42e68be0_set(0), half_42e68be2_set(1).", """
void table_scatter_42cf64b4(void);
void fonts_init(void);
void byte_42e68be0_set(u32 a);
void half_42e68be2_set(u32 a);

void draw_state_defaults(void)
{
    table_scatter_42cf64b4();
    fonts_init();
    byte_42e68be0_set(0);
    half_42e68be2_set(1);
}
""", src="display")
w(0x400344E0, 44, "display_start",
  "draw_state_defaults, colour 0x00FFFFFF, word_42e68c74_set(0x42E64A98), lcd_start, fb_clear_all.", """
void draw_state_defaults(void);
void draw_colour_set(u32 c);
void word_42e68c74_set(u32 a);
void lcd_start(void);
void fb_clear_all(void);

void display_start(void)
{
    draw_state_defaults();
    draw_colour_set(0x00FFFFFFu);
    word_42e68c74_set(0x42E64A98u);
    lcd_start();
    fb_clear_all();
}
""", src="display")

# ---------------------------------------------------------------- one-byte FPGA commands
for a, sel, desc, expr in (
        (0x400635D0, 0x61, "SYS+0x3B", "SYS8(0x3B)"),
        (0x40063460, 0x16, "SYS+0x27B", "SYS8(0x27B)"),
        (0x4004EEEC, 0x63, "byte 0x42E428E4", "*(volatile unsigned char *)0x42E428E4u")):
    w(a, 36, "fpga_cmd%02x_write" % sel, "Selector 0x%02X, then the data byte %s." % (sel, desc), """
void fpga_cmd%02x_write(void)
{
    fpga_selector_command_write(0x%02X);
    fpga_data_byte_write(%s);
}
""" % (sel, sel, expr), src="acquisition")
w(0x40063430, 48, "fpga_cmd15_write", "Selector 0x15, then 1 when SYS+0x27C is set, else 2.", """
void fpga_cmd15_write(void)
{
    fpga_selector_command_write(0x15);
    fpga_data_byte_write(SYS8(0x27C) ? 1 : 2);
}
""", src="acquisition")
w(0x40063354, 56, "fpga_cmd0c_write", "Selector 0x0C, then the u16 SYS+0x276, high byte first.", """
void fpga_cmd0c_write(void)
{
    fpga_selector_command_write(0x0C);
    fpga_data_byte_write(SYS16(0x276) >> 8);
    fpga_data_byte_write(SYS16(0x276) & 0xFF);
}
""", src="acquisition")
w(0x4006366C, 80, "fpga_cmd1a_write",
  "Selector 0x1A, then 1 when SYS+0x275, +0x274 and +0x27A are all set, else 0.", """
void fpga_cmd1a_write(void)
{
    u32 v = 0;
    fpga_selector_command_write(0x1A);
    if (SYS8(0x275) && SYS8(0x274) && SYS8(0x27A))
        v = 1;
    fpga_data_byte_write(v);
}
""", src="acquisition")
w(0x400636BC, 88, "fpga_cmd1b_write",
  "Selector 0x1B, then bits 8..15 and 0..7 of the word SYS+0x288 when SYS+0x275 and +0x274 are set, else two zero bytes.", """
void fpga_cmd1b_write(void)
{
    fpga_selector_command_write(0x1B);
    if (SYS8(0x275) && SYS8(0x274)) {
        fpga_data_byte_write((SYS32(0x288) >> 8) & 0xFF);
        fpga_data_byte_write(SYS32(0x288) & 0xFF);
    } else {
        fpga_data_byte_write(0);
        fpga_data_byte_write(0);
    }
}
""", src="acquisition")
w(0x400635F4, 52, "fpga_cmd61_pulse",
  "SYS+0x3B = 1 and fpga_cmd61_write; selector 0x17 with data 0; SYS+0x3B = 0 and fpga_cmd61_write.", """
void fpga_cmd61_write(void);

void fpga_cmd61_pulse(void)
{
    SYS8(0x3B) = 1;
    fpga_cmd61_write();
    fpga_selector_command_write(0x17);
    fpga_data_byte_write(0);
    SYS8(0x3B) = 0;
    fpga_cmd61_write();
}
""", src="acquisition")
w(0x40063628, 68, "fpga_cmd17_write",
  "When SYS+0x275 and +0x274 are set: selector 0x17 and the low byte of the u16 SYS+0x280; else fpga_cmd61_pulse.", """
void fpga_cmd61_pulse(void);

void fpga_cmd17_write(void)
{
    if (SYS8(0x275) && SYS8(0x274)) {
        fpga_selector_command_write(0x17);
        fpga_data_byte_write(SYS16(0x280) & 0xFF);
    } else {
        fpga_cmd61_pulse();
    }
}
""", src="acquisition")
w(0x40063770, 40, "fpga_cmd18_read", "Selector 0x18, then SYS+0x279 = bit 0 of one data byte read.", """
void fpga_cmd18_read(void)
{
    fpga_selector_command_write(0x18);
    SYS8(0x279) = fpga_data_byte_read() & 1;
}
""", src="acquisition")
w(0x40063328, 44, "fpga_config_push",
  "fpga_cmd61_write, fpga_cmd0c_write, fpga_cmd1a_write, fpga_cmd15_write, fpga_cmd16_write, 0x400634F0, then trigger_offsets_update.", """
void fpga_cmd61_write(void);
void fpga_cmd0c_write(void);
void fpga_cmd1a_write(void);
void fpga_cmd15_write(void);
void fpga_cmd16_write(void);
void fpga_params_0e_13_write(void);
void trigger_offsets_update(void);

void fpga_config_push(void)
{
    fpga_cmd61_write();
    fpga_cmd0c_write();
    fpga_cmd1a_write();
    fpga_cmd15_write();
    fpga_cmd16_write();
    fpga_params_0e_13_write();
    trigger_offsets_update();
}
""", src="acquisition")
w(0x4004F59C, 36, "fpga_probe_and_copy_7fc",
  "perform_fpga_identification_probe, then byte 0x42E68AE0+0x11 = SYS+0x7FC.", """
void perform_fpga_identification_probe(void);

void fpga_probe_and_copy_7fc(void)
{
    perform_fpga_identification_probe();
    *(volatile unsigned char *)0x42E68AF1u = SYS8(0x7FC);
}
""", src="acquisition")
w(0x4004F5C0, 36, "security_check_and_copy",
  "security_chip_check, then byte 0x42E68AE0+0x12 = byte 0x42E62C2C+0x8F8.", """
void security_chip_check(void);

void security_check_and_copy(void)
{
    security_chip_check();
    *(volatile unsigned char *)0x42E68AF2u = *(volatile unsigned char *)(0x42E62C2Cu + 0x8F8u);
}
""", src="acquisition")
w(0x40064DC0, 52, "check_pair_save",
  "0x40063A50; r = 0x40064C28() & 0x40064CF0() & 1; settings_a_save when r; 0x4003EB40; byte 0x42E68AE0+0x45 = r.", """
void table_684_init(void);
u32 gen_selftest_64c28(void);
u32 gen_selftest_64cf0(void);
void settings_a_save(void);
void func_4003eb40(void);

void check_pair_save(void)
{
    u32 a, r;
    table_684_init();
    a = gen_selftest_64c28();
    r = gen_selftest_64cf0() & a & 1;
    if (r)
        settings_a_save();
    func_4003eb40();
    *(volatile unsigned char *)0x42E68B25u = r;
}
""", src="init")

# ---------------------------------------------------------------- loose wrappers
w(0x400525D4, 24, "touch_00f2_then_400545f8", "touch_button_update_00F2, then 0x400545F8.", """
void touch_button_update_00F2(void);
void func_400545f8(void);

void touch_00f2_then_400545f8(void)
{
    touch_button_update_00F2();
    func_400545f8();
}
""")
w(0x40063CB0, 32, "func_40063c34_all8", "0x40063C34(i) for i = 0..7.", """
void fpga_counter_step(u32 i);

void func_40063c34_all8(void)
{
    u32 i;
    for (i = 0; i < 8; i++)
        fpga_counter_step(i & 0xFF);
}
""", src="acquisition")
for a, flag in ((0x4002D6D4, 1), (0x4002D6F8, 0)):
    w(a, 36, "func_4002d82c_flag%d" % flag,
      "0x4002D82C(a, b, c, d, the low byte of the fifth (stack) argument, %d)." % flag, """
void number_draw(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g);

void func_4002d82c_flag%d(u32 a, u32 b, u32 c, u32 d, u32 e)
{
    number_draw(a, b, c, d, e & 0xFF, %d);
}
""" % (flag, flag), src="display")
w(0x4002D71C, 8, "func_4002d2d4_d0", "0x4002D2D4(a, b, c, 0).", """
void number_text_width(u32 a, u32 b, u32 c, u32 d);

void func_4002d2d4_d0(u32 a, u32 b, u32 c)
{
    number_text_width(a, b, c, 0);
}
""", src="display")
w(0x40034058, 36, "func_40032e0c_both", "0x40032D3C, then 0x40032E0C(0) and 0x40032E0C(1).", """
void key_repeat_update(void);
void func_40032e0c(u32 a);

void func_40032e0c_both(void)
{
    key_repeat_update();
    func_40032e0c(0);
    func_40032e0c(1);
}
""")
w(0x400301A8, 56, "value_in_table_405456e8",
  "1 when v (negated when mode is 1) equals one of the 24 s16 at 0x405456E8, else 0.", """
u32 value_in_table_405456e8(u32 mode, int v)
{
    volatile short *t = (volatile short *)0x405456E8u;
    int n;
    if (mode == 1)
        v = -v;
    for (n = 24; n; n--)
        if (*t++ == v)
            return 1;
    return 0;
}
""")
w(0x400367A4, 44, "sys_3c_enter",
  "SYS+0x3D = SYS+0x3C; byte SYS+0x3DA52C = 1; SYS+0x3C = 1; SYS+0x652 = 0; SYS+0x2AA = 0.", """
void sys_3c_enter(void)
{
    SYS8(0x3D) = SYS8(0x3C);
    SYS8(0x3DA52C) = 1;
    SYS8(0x3C) = 1;
    SYS8(0x652) = 0;
    SYS8(0x2AA) = 0;
}
""")
w(0x400367D0, 28, "sys_3c_leave", "Byte SYS+0x3DA52C = 0; SYS+0x3C = SYS+0x3D.", """
void sys_3c_leave(void)
{
    u32 v = SYS8(0x3D);
    SYS8(0x3DA52C) = 0;
    SYS8(0x3C) = v;
}
""")
w(0x400373E0, 56, "reload_chain_400373e0",
  "0x40036B00, 0x40036BDC, 0x4003709C, 0x40036864, 0x40036DE4, 0x40036D88(u16 at 0x40545E04), 0x40036C54, 0x40036D10.", """
void list_c_index_load(void);
void list_c_data_load(void);
void list_c_next_free(void);
void func_40036864(void);
void func_40036de4(void);
void list_fa4_insert_758(u32 a);
void list_c_index_save(void);
void list_c_data_save(void);

void reload_chain_400373e0(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_next_free();
    func_40036864();
    func_40036de4();
    list_fa4_insert_758(*(volatile unsigned short *)0x40545E04u);
    list_c_index_save();
    list_c_data_save();
}
""")
w(0x400378E0, 56, "reload_chain_400378e0",
  "0x40036B00, 0x40036BDC, 0x400377F8, 0x4003771C, then SYS+0xF90 = 0, u16 SYS+0xFA4 = 0, u16 SYS+0xF9E = 0.", """
void list_c_index_load(void);
void list_c_data_load(void);
void list_c_marks_clear(void);
void list_cursor_clamp_4003771c(void);

void reload_chain_400378e0(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_marks_clear();
    list_cursor_clamp_4003771c();
    SYS8(0xF90) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
}
""")
w(0x40035744, 60, "reload_chain_40035744",
  "0x400358B8, 0x400363CC, SYS+0xF90 = 0, u16 SYS+0xFA4 = 0, u16 SYS+0xF9E = 0, then 0x400362F4, 0x4003656C, 0x400365F4.", """
void list_b_index_load(void);
void list_b_marks_clear(void);
void list_cursor_clamp_400362f4(void);
void list_b_page_validate(void);
void list_b_page_thumbs(void);

void reload_chain_40035744(void)
{
    list_b_index_load();
    list_b_marks_clear();
    SYS8(0xF90) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
    list_cursor_clamp_400362f4();
    list_b_page_validate();
    list_b_page_thumbs();
}
""")
