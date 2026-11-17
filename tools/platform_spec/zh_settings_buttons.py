# Touch-button handlers of the channel settings, list windows, system settings, settings slots,
# trigger options and the numpad fields of the measurement block.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wh(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


# ui_window_close / ui_window_open take one argument. ui_window_invalidate_from uses r1 only when the
# window is not on the stack: the stock callers pass whatever r1 holds, so the C passes the value the
# stock code leaves there, and inv_left() (same function, one argument) where it is a callee's leftover.
TBH = '''#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
void inv_left(u32 id) __asm__("ui_window_invalidate_from");
'''

CH = '''void outputs_apply(void);
void ui_windows_invalidate_1_5(void);

/* b = first button (toggle), s = SYS offset of the channel enable byte */
static void channel_buttons(u32 b, u32 s)
{
    if (TB(b) == 2) {
        TB(b) = 0;
        SYS8(s) = SYS8(s) == 0;
        if (SYS8(0x652))
            SYS8(s) = 1;
        TB(0) = 0;
    } else if (TB(b + 0xA) == 2) {
        TB(b + 0xA) = 0;
        TB(0) = 0;
        SYS8(s + 2) = 0;
    } else if (TB(b + 0x14) == 2) {
        TB(b + 0x14) = 0;
        TB(0) = 0;
        SYS8(s + 2) = 1;
    } else if (TB(b + 0x1E) == 2) {
        TB(b + 0x1E) = 0;
        TB(0) = 0;
        SYS8(s + 2) = 2;
    } else if (TB(b + 0x28) == 2) {
        TB(b + 0x28) = 0;
        TB(0) = 0;
        SYS8(s + 1) = 0;
    } else if (TB(b + 0x32) == 2) {
        TB(b + 0x32) = 0;
        TB(0) = 0;
        SYS8(s + 1) = 1;
    } else if (TB(b + 0x3C) == 2) {
        TB(b + 0x3C) = 0;
        SYS8(s + 4) = SYS8(s + 4) == 0;
        TB(0) = 0;
    } else if (TB(b + 0x5A) == 2) {
        TB(b + 0x5A) = 0;
        TB(0) = 0;
        SYS8(s + 8) = 0;
    } else if (TB(b + 0x46) == 2) {
        TB(b + 0x46) = 0;
        TB(0) = 0;
        SYS8(s + 8) = 1;
    } else if (TB(b + 0x50) == 2) {
        TB(b + 0x50) = 0;
        TB(0) = 0;
        SYS8(s + 8) = 2;
    } else {
        return;
    }
    outputs_apply();
    ui_windows_invalidate_1_5();
}
'''

wh(0x40052FC4, 488, "channel1_settings_buttons",
   "Channel 1 settings window, buttons 0x35E..0x3B8: toggle the channel (SYS+0x3E; kept on while SYS+0x652 is set), SYS+0x40 = 0..2, SYS+0x3F = 0..1, toggle SYS+0x42, SYS+0x46 = 0..2 (buttons 0x3B8, 0x3A4, 0x3AE in that order); then outputs_apply and ui_windows_invalidate_1_5.", TBH + CH + """
void touch_group_035E(void);

void channel1_settings_buttons(void)
{
    touch_group_035E();
    channel_buttons(0x35E, 0x3E);
}
""")

wh(0x400531B0, 496, "channel2_settings_buttons",
   "Channel 2 settings window, the same layout 0x64 buttons and 0xC4 SYS bytes further: buttons 0x3C2..0x41C, SYS+0x102, +0x104, +0x103, +0x106, +0x10A.", TBH + CH + """
void touch_group_03C2(void);

void channel2_settings_buttons(void)
{
    touch_group_03C2();
    channel_buttons(0x3C2, 0x102);
}
""")

wh(0x40054F50, 448, "list_a_window_buttons",
   "List A window: any of the buttons 0x818, 0x822, 0x82C, 0x836, 0x80E pressed (state 1) invalidates from window 0x16. Released: 0x818 closes windows 0x17 and 0x16, redraws the thumbnails and opens 0x14; 0x822 clears the marks, marks the current entry (SYS+0x5DC6 + s16 SYS+0xFA4), counts (0x400353A8) and opens 0x15; 0x82C / 0x836 move the current entry SYS+0xFA4 by -1 / +1 (page SYS+0xF9E = entry / page size SYS+0xF9A, sdiv) and reload it; 0x80E closes window 0x17. All but 0x818 then invalidate from window 0x16.", TBH + """
void touch_group_080E(void);
void list_a_page_thumbs(void);
void list_a_marks_clear(void);
void list_count_nonzero_400353a8(void);
void list_cursor_clamp_40035248(void);
void list_a_current_load(void);

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void list_a_window_buttons(void)
{
    u32 b822, b836;
    touch_group_080E();
    if (TB(0x818) == 1) {
        inv_left(0x16);
    } else if ((b822 = TB(0x822)) == 1) {
        ui_window_invalidate_from(0x16, 1);
    } else if (TB(0x82C) == 1) {
        ui_window_invalidate_from(0x16, b822);
    } else if ((b836 = TB(0x836)) == 1) {
        ui_window_invalidate_from(0x16, 1);
    } else if (TB(0x80E) == 1) {
        ui_window_invalidate_from(0x16, b836);
    }
    if (TB(0x818) == 2) {
        TB(0x818) = 0;
        TB(0) = 0;
        ui_window_close(0x17);
        ui_window_close(0x16);
        list_a_page_thumbs();
        ui_window_open(0x14);
        return;
    }
    if (TB(0x822) == 2) {
        TB(0x822) = 0;
        TB(0) = 0;
        list_a_marks_clear();
        SYS8(0x5DC6 + (short)SYS16(0xFA4)) = 1;
        list_count_nonzero_400353a8();
        ui_window_open(0x15);
    } else if (TB(0x82C) == 2 || TB(0x836) == 2) {
        int d;
        short v;
        if (TB(0x82C) == 2) {
            TB(0x82C) = 0;
            d = -1;
        } else {
            TB(0x836) = 0;
            d = 1;
        }
        TB(0) = 0;
        v = (short)((short)SYS16(0xFA4) + d);
        SYS16(0xFA4) = v;
        SYS16(0xF9E) = sdv(v, SYS16(0xF9A));
        list_cursor_clamp_40035248();
        list_a_current_load();
    } else if (TB(0x80E) == 2) {
        TB(0x80E) = 0;
        TB(0) = 0;
        ui_window_close(0x17);
    } else {
        return;
    }
    inv_left(0x16);
}
""")

wh(0x40055114, 560, "list_b_window_buttons",
   "List B window. Entry buttons 0x840 + 10i (i below the page size SYS+0xF9A): select entry page * size + i of SYS+0xFA4; when it exists (below SYS+0xFA0 + 1) it toggles its mark (SYS+0x5DC6 + entry) in mark mode SYS+0xF90, else opens it full screen. 0x8D6 back to the main screens; 0x8E0 toggles mark mode (marks cleared); 0x8EA marks the page or leaves mark mode; 0x8F4 opens the delete window 0x19 when marking; 0x8FE / 0x908 page -/+. Most paths end by invalidating from window 0x18.", TBH + """
void touch_buttons_840_update(void);
void ui_windows_reset(void);
void list_b_current_load(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_b_marks_clear(void);
void list_b_page_mark(void);
void list_count_nonzero_40036454(void);
void list_cursor_clamp_400362f4(void);
void list_b_page_thumbs(void);
#define P16(o) (*(volatile unsigned short *)(0x40544E68u + 0xF90 + (o)))
#define PS16(o) (*(volatile short *)(0x40544E68u + 0xF90 + (o)))

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void list_b_window_buttons(void)
{
    u32 i;
    int nv;
    touch_buttons_840_update();
    if (P16(0xA)) {
        i = 0;
        do {
            if (TB(0x840 + 10 * i) == 2) {
                int ps, tot, c;
                TB(0x840 + 10 * i) = 0;
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
                        ui_windows_reset();
                        list_b_current_load();
                        fb_clear_current();
                        ui_windows_open_1_5();
                    }
                }
            }
            i = (i + 1) & 0xFF;
        } while (i < P16(0xA));
    }
    if (TB(0x8D6) == 2) {
        TB(0x8D6) = 0;
        TB(0) = 0;
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        return;
    }
    if (TB(0x8E0) == 2) {
        TB(0x8E0) = 0;
        TB(0) = 0;
        SYS8(0xF90) = SYS8(0xF90) == 0;
        list_b_marks_clear();
    } else if (TB(0x8EA) == 2) {
        u32 v;
        TB(0x8EA) = 0;
        TB(0) = 0;
        v = SYS8(0x84D6 + PS16(0xE));
        if (SYS8(0xF90) == 0) {
            if (v) {
                ui_window_invalidate_from(0x18, v);
                return;
            }
            SYS8(0xF90) = 1;
            list_b_page_mark();
        } else if (v == 0) {
            list_b_page_mark();
        } else {
            SYS8(0xF90) = 0;
            list_b_marks_clear();
        }
    } else if (TB(0x8F4) == 2) {
        TB(0x8F4) = 0;
        TB(0) = 0;
        if (SYS8(0xF90)) {
            list_count_nonzero_40036454();
            ui_window_open(0x19);
        }
        return;
    } else {
        if (TB(0x8FE) == 2) {
            TB(0x8FE) = 0;
            TB(0) = 0;
            nv = PS16(0x14) - P16(0xA);
        } else if (TB(0x908) == 2) {
            TB(0x908) = 0;
            TB(0) = 0;
            nv = P16(0xA) + PS16(0x14);
        } else {
            return;
        }
        PS16(0x14) = (short)nv;
        PS16(0xE) = (short)sdv((short)nv, P16(0xA));
        list_cursor_clamp_400362f4();
        list_b_page_thumbs();
    }
    inv_left(0x18);
}
""")

wh(0x400525EC, 468, "system_settings_buttons",
   "System window: buttons 0xFC, 0x17E open windows 7 and 0x1D; 0x192 resets the windows, opens 0x20 and enters USB mode; 0x1A6, 0x1B0, 0x1BA, 0x1C4, 0x1CE set the auto-off word SYS+0x658 to 0, 15, 30, 45, 60 (then auto_off_arm and settings_b_save); 0x1D8, 0x1E2, 0x1F6 open windows 9, 0x1E, 0x1F.", TBH + """
void touch_group_017E(void);
void ui_windows_reset(void);
void usb_mode_enter(void);
void auto_off_arm(void);
void settings_b_save(void);

void system_settings_buttons(void)
{
    u32 w, i;
    touch_group_017E();
    if (TB(0xFC) == 2) {
        TB(0xFC) = 0;
        TB(0) = 0;
        w = 7;
    } else if (TB(0x17E) == 2) {
        TB(0x17E) = 0;
        TB(0) = 0;
        w = 0x1D;
    } else if (TB(0x192) == 2) {
        TB(0x192) = 0;
        TB(0) = 0;
        ui_windows_reset();
        ui_window_open(0x20);
        usb_mode_enter();
        return;
    } else {
        for (i = 0; i < 5; i++) {
            if (TB(0x1A6 + 10 * i) == 2) {
                TB(0x1A6 + 10 * i) = 0;
                TB(0) = 0;
                SYS32(0x658) = 15 * i;
                auto_off_arm();
                settings_b_save();
                return;
            }
        }
        if (TB(0x1D8) == 2) {
            TB(0x1D8) = 0;
            TB(0) = 0;
            w = 9;
        } else if (TB(0x1E2) == 2) {
            TB(0x1E2) = 0;
            TB(0) = 0;
            w = 0x1E;
        } else if (TB(0x1F6) == 2) {
            TB(0x1F6) = 0;
            TB(0) = 0;
            w = 0x1F;
        } else {
            return;
        }
    }
    ui_window_open(w);
}
""")

wh(0x40055798, 476, "settings_slot_buttons",
   "Settings slot window: buttons 0x124 + 10i set the slot SYS+0x7C0 = i + 1 (1..5) and, by the window mode byte 0x42E64587 (1 save, 2 load, 3 save block F then load), run settings_slot_save / settings_slot_load; then the message 0xAAA0 at the centre of window rectangle 0x76 for 500 ms, closes window 8, applies outputs and the generator and marks the windows dirty.", TBH + """
void touch_group_0124(void);
void settings_slot_save(void);
void settings_slot_load(void);
void settings_f_save(void);
void message_box_draw(int x, int y, u32 text, int align);
void delay_ms(u32 ms);
void rect_42e68c38_draw(void);
void outputs_apply(void);
void gen_update(void);
void ui_windows_mark_dirty(void);
#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define WR16(o) (*(volatile unsigned short *)(0x42E64520u + (o)))

void settings_slot_buttons(void)
{
    u32 cx, cy, i, m;
    touch_group_0124();
    cx = (unsigned short)(WR16(0x76) + (WR16(0x7A) >> 1));
    cy = (unsigned short)(WR16(0x78) + (WR16(0x7C) >> 1));
    for (i = 0; i < 5; i++)
        if (TB(0x124 + 10 * i) == 2)
            break;
    if (i == 5)
        return;
    TB(0x124 + 10 * i) = 0;
    TB(0) = 0;
    SYS8(0x7C0) = i + 1;
    m = *(volatile unsigned char *)(0x42E64520u + 0x67);
    if (m == 1) {
        settings_slot_save();
    } else if (m == 2) {
        settings_slot_load();
    } else if (m == 3) {
        settings_f_save();
        settings_slot_load();
    }
    message_box_draw((int)cx + TX8(0xAAA0 + 0x194), (int)cy + TX8(0xAAA0 + 0x195), TX32(0xAAA0), 0);
    delay_ms(500);
    rect_42e68c38_draw();
    ui_window_close(8);
    outputs_apply();
    gen_update();
    ui_windows_mark_dirty();
}
""")

wh(0x40053940, 552, "trigger_option_buttons",
   "Trigger options window, buttons 0x5B6 + 10i: SYS+0x27A = 0..2 (the second also closes window 0x11), SYS+0x27B = 0..1, SYS+0x27C = 0..1, SYS+0x284 = 0..3; each invalidates from window 3 (r1 as the stock leaves it). Byte SYS+0x284 is rewritten when none of its buttons fired.", TBH + """
void touch_group_05B6(void);

void trigger_option_buttons(void)
{
    u32 v;
    touch_group_05B6();
    if (TB(0x5B6) == 2) {
        TB(0x5B6) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5B6);
        SYS8(0x27A) = 0;
    } else if (TB(0x5C0) == 2) {
        TB(0x5C0) = 0;
        TB(0) = 0;
        ui_window_close(0x11);
        inv_left(3);
        SYS8(0x27A) = 1;
    } else if (TB(0x5CA) == 2) {
        TB(0x5CA) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SYS8(0x27A) = 2;
    } else if (TB(0x5D4) == 2) {
        TB(0x5D4) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5D4);
        SYS8(0x27B) = 0;
    } else if (TB(0x5DE) == 2) {
        TB(0x5DE) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SYS8(0x27B) = 1;
    } else if (TB(0x5E8) == 2) {
        TB(0x5E8) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0x5E8);
        SYS8(0x27C) = 0;
    } else if (TB(0x5F2) == 2) {
        TB(0x5F2) = 0;
        TB(0) = 0;
        ui_window_invalidate_from(3, 0);
        SYS8(0x27C) = 1;
    } else {
        if (TB(0x5FC) == 2) {
            TB(0x5FC) = 0;
            TB(0) = 0;
            ui_window_invalidate_from(3, 0);
            v = 0;
        } else {
            v = SYS8(0x284);
            if (TB(0x606) == 2) {
                TB(0x606) = 0;
                TB(0) = 0;
                ui_window_invalidate_from(3, 0);
                v = 1;
            } else if (TB(0x610) == 2) {
                TB(0x610) = 0;
                TB(0) = 0;
                ui_window_invalidate_from(3, 0);
                v = 2;
            } else if (TB(0x61A) == 2) {
                TB(0x61A) = 0;
                TB(0) = 0;
                ui_window_invalidate_from(3, 0);
                v = 3;
            }
        }
        SYS8(0x284) = v;
    }
}
""")

wh(0x40056530, 500, "measure_numpad_buttons",
   "Measurement window numpad fields, buttons 0xB92..0xBC4: each clears byte 0x42E68A8A and opens the numpad (mode 3 at x = 2 / 0x92, mode 1 at 0x124, 0x1B6, 0x248, 0x28B; y = 0xE9) with the callback 0x400617AC, flag 0x42E68AD4 = 1 and a target in block 0x42E68668: the doubles +0x148 / +0x140 (word 0x42E68AB0), or the s64 +0x120, +0x128, +0x130, +0x138 (word 0x42E68AB4, with mode +0x18C = 1..4). Button 0xBCE toggles byte +3. Except for the first two, the four target words are always rewritten.", TBH + """
void touch_group_0B92(void);
void numpad_open_mode1(u32 x, u32 y);
void numpad_open_mode3(u32 x, u32 y);
#define NP8(o) (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define NP32(o) (*(volatile u32 *)(0x42E68A88u + (o)))
#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define CB 0x400617ACu

void measure_numpad_buttons(void)
{
    u32 fl, cb, tgt, mode;
    touch_group_0B92();
    if (TB(0xB92) == 2) {
        TB(0xB92) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode3(2, 0xE9);
        NP32(0x28) = 0x42E68668u + 0x148;
        NP32(0x50) = CB;
        NP8(0x4C) = 1;
        return;
    }
    fl = NP8(0x4C);
    cb = NP32(0x50);
    if (TB(0xB9C) == 2) {
        TB(0xB9C) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode3(0x92, 0xE9);
        NP32(0x28) = 0x42E68668u + 0x140;
        NP32(0x50) = CB;
        NP8(0x4C) = 1;
        return;
    }
    if (TB(0xBA6) == 2) {
        TB(0xBA6) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode1(0x124, 0xE9);
        tgt = 0x42E68668u + 0x120, fl = 1, cb = CB, mode = 1;
    } else {
        tgt = NP32(0x2C);
        mode = MB32(0x18C);
        if (TB(0xBB0) == 2) {
            TB(0xBB0) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x1B6, 0xE9);
            tgt = 0x42E68668u + 0x128, fl = 1, cb = CB, mode = 2;
        } else if (TB(0xBBA) == 2) {
            TB(0xBBA) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x248, 0xE9);
            tgt = 0x42E68668u + 0x130, fl = 1, cb = CB, mode = 3;
        } else if (TB(0xBC4) == 2) {
            TB(0xBC4) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x28B, 0xE9);
            tgt = 0x42E68668u + 0x138, fl = 1, cb = CB, mode = 4;
        } else if (TB(0xBCE) == 2) {
            TB(0xBCE) = 0;
            TB(0) = 0;
            MB8(3) = MB8(3) == 0;
        }
    }
    NP32(0x2C) = tgt;
    MB32(0x18C) = mode;
    NP32(0x50) = cb;
    NP8(0x4C) = fl;
}
""")
