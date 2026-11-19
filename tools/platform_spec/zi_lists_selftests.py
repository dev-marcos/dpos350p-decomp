# Touch-button handlers of the list A thumbnail and list C windows, two settings windows, the
# numpad key buttons, and two generator / measurement self-tests.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wi(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


TBI = '''#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
void inv_left(u32 id) __asm__("ui_window_invalidate_from");
'''

LIST_COMMON = '''#define P16(o) (*(volatile unsigned short *)(0x40544E68u + 0xF90 + (o)))
#define PS16(o) (*(volatile short *)(0x40544E68u + 0xF90 + (o)))

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}
'''


def list_window(name, update, e, back, open_entry, marks_clear, page_mark, delete, page, win):
    """C of a list window handler: entry buttons e + 10i, then back, back+0xA (mark mode),
    back+0x14 (mark page), back+0x1E (delete), back+0x28 / back+0x32 (page - / +)."""
    return TBI + LIST_COMMON + '''
void %(update)s(void);

void %(name)s(void)
{
    u32 i;
    int nv;
    %(update)s();
    if (P16(0xA)) {
        i = 0;
        do {
            if (TB(0x%(e)X + 10 * i) == 2) {
                int ps, tot, c;
                TB(0x%(e)X + 10 * i) = 0;
                TB(0) = 0;
                ps = P16(0xA);
                tot = PS16(0x10);
                PS16(0x14) = (short)(ps * sdv(PS16(0x14), ps) + (int)i);
                c = PS16(0x14);
                if (tot >= c + 1) {
                    if (SYS8(0xF90)) {
                        volatile unsigned char *m = (volatile unsigned char *)(0x40544E68u + 0x5DC6 + c);
                        *m = *m == 0;
                    } else {
%(open_entry)s
                    }
                }
            }
            i = (i + 1) & 0xFF;
        } while (i < P16(0xA));
    }
    if (TB(0x%(b0)X) == 2) {
        TB(0x%(b0)X) = 0;
        TB(0) = 0;
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        return;
    }
    if (TB(0x%(b1)X) == 2) {
        TB(0x%(b1)X) = 0;
        TB(0) = 0;
        SYS8(0xF90) = SYS8(0xF90) == 0;
        %(marks_clear)s();
    } else if (TB(0x%(b2)X) == 2) {
        u32 v;
        TB(0x%(b2)X) = 0;
        TB(0) = 0;
        v = SYS8(0x84D6 + PS16(0xE));
        if (SYS8(0xF90) == 0) {
            if (v) {
                ui_window_invalidate_from(0x%(win)X, v);
                return;
            }
            SYS8(0xF90) = 1;
            %(page_mark)s();
        } else if (v == 0) {
            %(page_mark)s();
        } else {
            SYS8(0xF90) = 0;
            %(marks_clear)s();
        }
    } else if (TB(0x%(b3)X) == 2) {
        TB(0x%(b3)X) = 0;
        TB(0) = 0;
        if (SYS8(0xF90)) {
%(delete)s
        }
        return;
    } else {
        if (TB(0x%(b4)X) == 2) {
            TB(0x%(b4)X) = 0;
            TB(0) = 0;
            nv = PS16(0x14) - P16(0xA);
        } else if (TB(0x%(b5)X) == 2) {
            TB(0x%(b5)X) = 0;
            TB(0) = 0;
            nv = P16(0xA) + PS16(0x14);
        } else {
            return;
        }
        PS16(0x14) = (short)nv;
        PS16(0xE) = (short)sdv((short)nv, P16(0xA));
%(page)s
    }
    inv_left(0x%(win)X);
}
''' % dict(name=name, update=update, e=e, b0=back, b1=back + 0xA, b2=back + 0x14, b3=back + 0x1E,
           b4=back + 0x28, b5=back + 0x32, open_entry=open_entry, marks_clear=marks_clear,
           page_mark=page_mark, delete=delete, page=page, win=win)


wi(0x40054C84, 564, "list_a_thumbs_buttons",
   "List A thumbnail window: entry buttons 0x728 + 10i (i below the page size SYS+0xF9A) select entry page * size + i of SYS+0xFA4; when it exists (below SYS+0xFA0 + 1) it toggles its mark (SYS+0x5DC6 + entry) in mark mode SYS+0xF90, else loads it and opens window 0x16. 0x7BE back to the main screens; 0x7C8 toggles mark mode (marks cleared); 0x7D2 marks the page or leaves mark mode; 0x7DC opens the delete window 0x15 when marking; 0x7E6 / 0x7F0 page -/+ (thumbnails redrawn). Most paths end by invalidating from window 0x14.",
   """
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_a_current_load(void);
void list_a_marks_clear(void);
void list_a_page_mark(void);
void list_count_nonzero_400353a8(void);
void list_cursor_clamp_40035248(void);
void list_a_page_thumbs(void);
""" + list_window("list_a_thumbs_buttons", "touch_buttons_728_update", 0x728, 0x7BE,
                  "                        ui_windows_reset();\n                        list_a_current_load();\n"
                  "                        ui_window_open(0x16);",
                  "list_a_marks_clear", "list_a_page_mark",
                  "            list_count_nonzero_400353a8();\n            ui_window_open(0x15);",
                  "        list_cursor_clamp_40035248();\n        list_a_page_thumbs();", 0x14))

wi(0x400553A0, 584, "list_c_window_buttons",
   "List C window, the same layout as list A / B: entry buttons 0x91C + 10i select an entry; outside mark mode the entry's record number (halfword SYS+0xFA6 + 2 * entry) goes to halfword 0x4091F396 (SYS+0x3DA52E), the index is saved, that record selected and the generator samples loaded and applied. 0x9B2 back; 0x9BC mark mode; 0x9C6 mark page; 0x9D0 opens the delete window 0x1B when marking; 0x9DA / 0x9E4 page -/+. Most paths end by invalidating from window 0x1A.",
   """
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_c_index_save(void);
void list_c_record_select(u32 v);
void gen_load_samples(void);
void gen_update(void);
void list_c_marks_clear(void);
void list_c_page_mark(void);
void list_count_nonzero_40037880(void);
void list_cursor_clamp_4003771c(void);
#define LC_REC (*(volatile unsigned short *)(0x40544E68u + 0x3DA52E))
""" + list_window("list_c_window_buttons", "touch_buttons_91c_update", 0x91C, 0x9B2,
                  "                        LC_REC = *(volatile unsigned short *)(0x40544E68u + 0xFA6 + 2 * c);\n"
                  "                        list_c_index_save();\n                        list_c_record_select(LC_REC);\n"
                  "                        gen_load_samples();\n                        gen_update();",
                  "list_c_marks_clear", "list_c_page_mark",
                  "            list_count_nonzero_40037880();\n            ui_window_open(0x1B);",
                  "        list_cursor_clamp_4003771c();", 0x1A))

wi(0x400527C0, 552, "buttons_20a_handler",
   "Window of button group 0x20A. While the slider byte 0x42E62203 is set: touch byte = 1 and SYS+0x654 = 100 * (x - lo) / (hi - lo) (sdiv), x = s16 0x42E68BB0 clamped to the halfwords lo = 0x42E62204, hi = 0x42E62208. Else buttons: 0x214 / 0x21E / 0x228 toggle SYS+0x653 / +0x652 / +0x655; 0x17E only clears; 0x232 closes windows 0xA, 5, 2, opens 0x1C and 5 and calls sys_3c_enter; 0x23C backs up the settings and generator state, reinitialises, and opens windows 0x22..0x24; 0x246 backs up the settings, opens windows 0x26..0x28 and sets SYS+0x54E = 1.",
   TBI + """
void touch_group_020A(void);
void sys_3c_enter(void);
void sys_info_backup(void);
void gen_state_backup(void);
void acq_defaults_reset(void);
void state_block_3f984_init(void);
void ui_windows_reset(void);
void fb_clear_current(void);
#define TH16(o) (*(volatile unsigned short *)(0x42E61FF8u + (o)))

void buttons_20a_handler(void)
{
    touch_group_020A();
    if (TB(0x20B)) {
        int x, lo, hi, d, n;
        TB(0) = 1;
        x = *(volatile short *)0x42E68BB0u;
        lo = TH16(0x20C);
        hi = TH16(0x210);
        if (x < lo)
            x = lo;
        else if (hi < x)
            x = hi;
        n = (int)((u32)(x - lo) * 100u);
        d = hi - lo;
        SYS8(0x654) = d == 0 ? 0 : d == -1 ? (int)(0u - (u32)n) : n / d;
        return;
    }
    if (TB(0x214) == 2) {
        TB(0) = 0;
        TB(0x214) = 0;
        SYS8(0x653) = SYS8(0x653) == 0;
    } else if (TB(0x21E) == 2) {
        TB(0) = 0;
        TB(0x21E) = 0;
        SYS8(0x652) = SYS8(0x652) == 0;
    } else if (TB(0x228) == 2) {
        TB(0) = 0;
        TB(0x228) = 0;
        SYS8(0x655) = SYS8(0x655) == 0;
    } else if (TB(0x17E) == 2) {
        TB(0x17E) = 0;
        TB(0) = 0;
    } else if (TB(0x232) == 2) {
        TB(0) = 0;
        TB(0x232) = 0;
        ui_window_close(0xA);
        ui_window_close(5);
        ui_window_close(2);
        ui_window_open(0x1C);
        ui_window_open(5);
        sys_3c_enter();
    } else if (TB(0x23C) == 2) {
        TB(0x23C) = 0;
        TB(0) = 0;
        sys_info_backup();
        gen_state_backup();
        acq_defaults_reset();
        state_block_3f984_init();
        ui_windows_reset();
        fb_clear_current();
        ui_window_open(0x22);
        ui_window_open(0x23);
        ui_window_open(0x24);
    } else if (TB(0x246) == 2) {
        TB(0x246) = 0;
        TB(0) = 0;
        sys_info_backup();
        acq_defaults_reset();
        ui_windows_reset();
        fb_clear_current();
        ui_window_open(0x26);
        ui_window_open(0x27);
        ui_window_open(0x28);
        SYS8(0x54E) = 1;
    }
}
""")

wi(0x40052280, 564, "buttons_05c_handler",
   "Window of button group 0x5C: 0x5C toggles SYS+0x3C (turning it off also clears SYS+0x665 and, if that was set, restores the saved state); 0x66 calls 0x4004C7C4; 0x70 resets the word pair 0x570 or 0x578 (by SYS+0x2AA) and sets SYS+0x27E, +0x60, +0x120 to 250; 0x7A / 0x8E step the byte SYS+0x41 (or SYS+0x105 when SYS+0x664 is set) up to SYS+0x1BA - 1 / down to 0, then apply the front end and invalidate from window 3; 0x84 toggles SYS+0x664 and invalidates from window 2; 0x98 calls 0x4006382C; 0xA2 opens window 0x13.",
   TBI + """
void touch_group_005C(void);
void state_restore_all(void);
void func_4004c7c4(void);
void sys_word_pair_570_set(u32 lo, u32 hi);
void sys_word_pair_578_set(u32 lo, u32 hi);
void frontend_apply_all(void);
void trigger_level_auto(void);

static void level_apply(void)
{
    TB(0) = 0;
    frontend_apply_all();
    inv_left(3);
}

void buttons_05c_handler(void)
{
    touch_group_005C();
    if (TB(0x5C) == 2) {
        TB(0x5C) = 0;
        TB(0) = 0;
        if (SYS8(0x3C) == 0) {
            SYS8(0x3C) = 1;
            return;
        }
        SYS8(0x3C) = 0;
        if (SYS8(0x665)) {
            SYS8(0x665) = 0;
            state_restore_all();
        }
    } else if (TB(0x66) == 2) {
        TB(0x66) = 0;
        TB(0) = 0;
        func_4004c7c4();
    } else if (TB(0x70) == 2) {
        TB(0x70) = 0;
        TB(0) = 0;
        if (SYS8(0x2AA) == 0)
            sys_word_pair_570_set(0, 0);
        else
            sys_word_pair_578_set(0, 0);
        SYS16(0x27E) = 0xFA;
        SYS32(0x60) = 0xFA;
        SYS32(0x120) = 0xFA;
    } else if (TB(0x7A) == 2) {
        int top;
        TB(0x7A) = 0;
        top = (int)SYS8(0x1BA) - 1;
        if (SYS8(0x664) == 0) {
            if ((int)SYS8(0x41) < top)
                SYS8(0x41) = SYS8(0x41) + 1;
        } else if ((int)SYS8(0x105) < top) {
            SYS8(0x105) = SYS8(0x105) + 1;
        }
        level_apply();
    } else if (TB(0x84) == 2) {
        u32 v;
        TB(0x84) = 0;
        v = SYS8(0x664) == 0;
        SYS8(0x664) = v;
        ui_window_invalidate_from(2, v);
    } else if (TB(0x8E) == 2) {
        TB(0x8E) = 0;
        if (SYS8(0x664) == 0) {
            if (SYS8(0x41))
                SYS8(0x41) = SYS8(0x41) - 1;
        } else if (SYS8(0x105)) {
            SYS8(0x105) = SYS8(0x105) - 1;
        }
        level_apply();
    } else if (TB(0x98) == 2) {
        TB(0x98) = 0;
        TB(0) = 0;
        trigger_level_auto();
    } else if (TB(0xA2) == 2) {
        TB(0xA2) = 0;
        TB(0) = 0;
        ui_window_open(0x13);
    }
}
""")

wi(0x40055CC4, 616, "numpad_key_buttons",
   "Numpad window, buttons 0xA20..0xAB6 (step 10): each sends its key code to numpad_key: 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 1, 2, 0xD, 0xE, 0xF, 0x10. The first (key 3) also invalidates from window 0x21.",
   TBI + """
void touch_buttons_a20_update(void);
void numpad_key(u32 key);

void numpad_key_buttons(void)
{
    u32 i, k;
    touch_buttons_a20_update();
    if (TB(0xA20) == 2) {
        TB(0xA20) = 0;
        TB(0) = 0;
        numpad_key(3);
        inv_left(0x21);
        return;
    }
    for (i = 1; i < 16; i++) {
        if (TB(0xA20 + 10 * i) == 2) {
            TB(0xA20 + 10 * i) = 0;
            TB(0) = 0;
            if (i < 10)
                k = i + 3;
            else if (i < 12)
                k = i - 9;
            else
                k = i + 1;
            numpad_key(k);
            return;
        }
    }
}
""")

SELFTEST = '''void acq_defaults_reset(void);
void outputs_apply(void);
void gen_update(void);
void delay_ms(u32 ms);
void acquisition_dispatcher(void);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define S64(o) (*(volatile long long *)(0x40544E68u + (o)))

/* one generator setting: amplitude word a, range byte b, level c, offset d; measure twice */
static void st_step(u32 a, u32 b, u32 c, u32 d, int wait)
{
    GB8(0x2EFA) = 0xD;
    GB32(0x2EFC) = 0;
    GB32(0x2F00) = 0;
    GB32(0x2F04) = a;
    SYS8(0x588) = b;
    SYS8(0x41) = c;
    SYS8(0x105) = c;
    SYS32(0x60) = d;
    SYS32(0x120) = d;
    outputs_apply();
    gen_update();
    if (wait)
        delay_ms(500);
    acquisition_dispatcher();
    acquisition_dispatcher();
}
'''

wi(0x400504B8, 464, "gen_selftest_504b8",
   "Self-test (DERIVED from the settings it writes): both channels on (SYS+0x3E, +0x102, +0x3F, +0x103 = 1), SYS+0x660 = 5, byte 0x42E68BC6 = 0; one generator setting (byte +0x2EFA = 0xD, words +0x2EFC, +0x2F00 = 0, +0x2F04 = 1000 of block 0x42E3F984; SYS+0x588 = 0x18, SYS+0x41 = SYS+0x105 = 6, SYS+0x60 = SYS+0x120 = 0), applied, 500 ms, two acquisitions. Pass flags: s64 SYS+0x8F8 / 1000 and SYS+0x958 / 1000 within -399..399, stored in bytes 0x42E68B08 / 0x42E68B09 and their AND in 0x42E68B07.",
   SELFTEST + """
void gen_selftest_504b8(void)
{
    u32 a, b;
    acq_defaults_reset();
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS8(0x3F) = 1;
    SYS8(0x103) = 1;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    st_step(1000, 0x18, 6, 0, 1);
    {
        int q1 = (int)(S64(0x958) / 1000);
        int q2 = (int)(S64(0x8F8) / 1000);
        a = (u32)(q2 + 0x18F) < 0x31Fu;
        b = (u32)(q1 + 0x18F) < 0x31Fu;
    }
    RS8(0x28) = a;
    RS8(0x29) = b;
    RS8(0x27) = b & a;
}
""", src="init")

wi(0x40050688, 520, "gen_selftest_50688",
   "Self-test (DERIVED): both channels on, SYS+0x660 = 5, byte 0x42E68BC6 = 0; two generator settings (as gen_selftest_504b8, no delay): amplitude +2500 with offset 0 and -2500 with offset halfword SYS+0xA. Pass flags per setting: s64 SYS+0x8F8 / 1000 and SYS+0x958 / 1000 at least 1500 (positive) / below -1499 (negative); ANDed over both settings into bytes 0x42E68B0F / 0x42E68B10, and together into 0x42E68B0E.",
   SELFTEST + """
void gen_selftest_50688(void)
{
    u32 a = 1, b = 1, i, off;
    acq_defaults_reset();
    off = SYS16(0xA);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    for (i = 0; i < 2; i++) {
        int amp = i ? -2500 : 2500;
        int q1, q2;
        st_step((u32)amp, 0x18, 6, i ? off : 0, 0);
        q1 = (int)(S64(0x8F8) / 1000);
        q2 = (int)(S64(0x958) / 1000);
        if (amp > 0) {
            a &= q1 >= 1500;
            b &= q2 >= 1500;
        } else {
            a &= q1 < -1499;
            b &= q2 < -1499;
        }
    }
    RS8(0x2F) = a;
    RS8(0x30) = b;
    RS8(0x2E) = b & a;
}
""", src="init")
