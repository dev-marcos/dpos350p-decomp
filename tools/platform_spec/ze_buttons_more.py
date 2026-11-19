# More touch-button handlers (cursor and marker drags, calibration, file and window buttons, numpad
# fields), the level update chain, the FPGA parameters 0x0E..0x13 and a generator self-test.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def we(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


# ui_window_close(id) is declared with one argument: the stock callers leave in r1/r2 whatever the
# preceding call returned there (ui_window_close only uses r2 when the window is not on the stack).
TBE = '''#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
'''
DGE = '''#define DG8(o) (*(volatile unsigned char *)(0x42E68998u + (o)))
#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))
#define DG64(o) (*(volatile long long *)(0x42E68998u + (o)))
'''
MSGE = '''void message_box_draw(int x, int y, u32 text, int align);
void delay_ms(u32 ms);
#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))
#define WR16(o) (*(volatile unsigned short *)(0x42E64520u + (o)))
#define MSG(cx, cy, b) message_box_draw((int)(cx) + TX8((b) + 0x194), (int)(cy) + TX8((b) + 0x195), TX32(b), 0)
'''

we(0x40056214, 220, "cursor_drag_buttons_b42",
   "Button 0xB42 pressed (state 1): touch_pick_b and, in drag mode 0xE / 0xF, keeps cursor word 0x42E22D2C / +4 as the 64-bit drag start. Byte 0xB43 = 1: touch_drag_delta_d and in those modes sets the cursor to start - word +0x30 of the drag block, then cursor_ac64_clamp.", TBE + DGE + """
void touch_button_update_0B42(void);
void touch_pick_b(void);
void touch_drag_delta_d(void);
void cursor_ac64_clamp(void);
#define CUR(o) (*(volatile int *)(0x42E22D2Cu + (o)))

void cursor_drag_buttons_b42(void)
{
    u32 m;
    touch_button_update_0B42();
    if (TB(0xB42) == 1) {
        TB(0xB42) = 0;
        TB(0) = 0;
        touch_pick_b();
        m = DG8(0);
        if (m == 0xE)
            DG64(8) = CUR(0);
        else if (m == 0xF)
            DG64(8) = CUR(4);
    }
    if (TB(0xB43) != 1)
        return;
    TB(0xB43) = 0;
    TB(0) = 1;
    touch_drag_delta_d();
    m = DG8(0);
    if (m == 0xE)
        CUR(0) = DG32(8) - DG32(0x30);
    else if (m == 0xF)
        CUR(4) = DG32(8) - DG32(0x30);
    else
        return;
    cursor_ac64_clamp();
}
""", src="trace")

we(0x40056724, 220, "marker_drag_buttons_bd8",
   "Button 0xBD8 pressed: touch_pick_a and, in drag mode 0x10 / 0x11, keeps word +0x5C / +0x60 of block 0x42E68668 as the 64-bit drag start. Byte 0xBD9 = 1: touch_drag_delta_c and in those modes sets that word to start - word +0x30 of the drag block.", TBE + DGE + """
void touch_button_update_0BD8(void);
void touch_pick_a(void);
void touch_drag_delta_c(void);
#define MK(o) (*(volatile int *)(0x42E68668u + (o)))

void marker_drag_buttons_bd8(void)
{
    u32 m;
    touch_button_update_0BD8();
    if (TB(0xBD8) == 1) {
        TB(0xBD8) = 0;
        TB(0) = 0;
        touch_pick_a();
        m = DG8(0);
        if (m == 0x10)
            DG64(8) = MK(0x5C);
        else if (m == 0x11)
            DG64(8) = MK(0x60);
    }
    if (TB(0xBD9) != 1)
        return;
    TB(0xBD9) = 0;
    TB(0) = 1;
    touch_drag_delta_c();
    m = DG8(0);
    if (m == 0x10)
        MK(0x5C) = DG32(8) - DG32(0x30);
    else if (m == 0x11)
        MK(0x60) = DG32(8) - DG32(0x30);
}
""", src="trace")

we(0x400562F4, 268, "calibration_b4c_button_handler",
   "Button 0xB4C released: message 0x9C00 at the centre of window rectangle 0x20C, runs 0x40039EB8, then message 0xD680 when byte 0x42E22DE1 or 0x42E22DE2 is 0, else message 0x9C00 + 0x1A0; waits 1 s and closes window 0x25.", TBE + MSGE + """
void touch_button_update_0B4C(void);
void bode_calibrate(void);
#define TT8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))

void calibration_b4c_button_handler(void)
{
    u32 cx, cy;
    touch_button_update_0B4C();
    cx = (unsigned short)(WR16(0x20C) + (WR16(0x210) >> 1));
    cy = (unsigned short)(WR16(0x20E) + (WR16(0x212) >> 1));
    if (TB(0xB4C) != 2)
        return;
    TB(0xB4C) = 0;
    TB(0) = 0;
    MSG(cx, cy, 0x9C00);
    bode_calibrate();
    if (TT8(0xAD19) == 0 || TT8(0xAD1A) == 0)
        MSG(cx, cy, 0xD680);
    else
        message_box_draw((int)cx + TX8(0x9C00 + 0x334), (int)cy + TX8(0x9C00 + 0x335), TX32(0x9C00 + 0x1A0), 0);
    delay_ms(1000);
    ui_window_close(0x25);
}
""")

we(0x40056404, 292, "buttons_b56_handler",
   "Buttons 0xB56 (restore SYS_INFO, reset and clear the screen, open windows 1 and 5, SYS+0x54E = 0, SYS+0x5F0 = 100000000), 0xB60 (open window 0x29), 0xB6A (save a screenshot), 0xB74 (toggle word +0x58 of block 0x42E68668), 0xB7E (measurement setup unless locked), 0xB88 (toggle byte +0 of that block).", TBE + """
void touch_group_0B56(void);
void sys_info_restore(void);
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_a_save_new(void);
void func_40062e14_unless_locked(void);
#define MK(o) (*(volatile u32 *)(0x42E68668u + (o)))

void buttons_b56_handler(void)
{
    touch_group_0B56();
    if (TB(0xB56) == 2) {
        TB(0xB56) = 0;
        TB(0) = 0;
        sys_info_restore();
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        SYS8(0x54E) = 0;
        SYS32(0x5F0) = 100000000;
    } else if (TB(0xB60) == 2) {
        TB(0xB60) = 0;
        TB(0) = 0;
        ui_window_open(0x29);
    } else if (TB(0xB6A) == 2) {
        TB(0xB6A) = 0;
        TB(0) = 0;
        list_a_save_new();
    } else if (TB(0xB74) == 2) {
        TB(0xB74) = 0;
        TB(0) = 0;
        MK(0x58) = MK(0x58) == 0;
    } else if (TB(0xB7E) == 2) {
        TB(0xB7E) = 0;
        TB(0) = 0;
        func_40062e14_unless_locked();
    } else if (TB(0xB88) == 2) {
        TB(0xB88) = 0;
        TB(0) = 0;
        *(volatile unsigned char *)0x42E68668u = *(volatile unsigned char *)0x42E68668u == 0;
    }
}
""")

we(0x400524B4, 288, "buttons_ac_open_windows",
   "Buttons 0xAC, 0xB6, 0xC0, 0xCA, 0xD4, 0xDE open windows 0xD..0x12; button 0xE8 toggles byte SYS+0x671 and invalidates from window 3 (r1 = the new value).", TBE + """
void touch_group_00AC(void);
void ui_window_invalidate_from(u32 id, u32 r1);

void buttons_ac_open_windows(void)
{
    u32 i, v;
    touch_group_00AC();
    for (i = 0; i < 6; i++) {
        if (TB(0xAC + 10 * i) == 2) {
            TB(0xAC + 10 * i) = 0;
            TB(0) = 0;
            ui_window_open(0xD + i);
            return;
        }
    }
    if (TB(0xE8) != 2)
        return;
    TB(0xE8) = 0;
    TB(0) = 0;
    v = SYS8(0x671) == 0;
    SYS8(0x671) = v;
    ui_window_invalidate_from(3, v);
}
""")

we(0x40055F2C, 340, "buttons_ac0_handler",
   "Buttons 0xAC0 (restore SYS_INFO and the generator state, reset and clear the screen, open windows 1 and 5), 0xACA (open window 0x25), 0xAD4 (save a screenshot), 0xADE / 0xAF2 (toggle bytes 0x42E22D28 / 0x42E1810C), 0xAFC (toggle byte 0x42E1810D, copied to 0x42E1810F).", TBE + """
void touch_group_0AC0(void);
void sys_info_restore(void);
void gen_state_restore(void);
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_a_save_new(void);
#define TT8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))

void buttons_ac0_handler(void)
{
    u32 v;
    touch_group_0AC0();
    if (TB(0xAC0) == 2) {
        TB(0xAC0) = 0;
        TB(0) = 0;
        sys_info_restore();
        gen_state_restore();
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
    } else if (TB(0xACA) == 2) {
        TB(0xACA) = 0;
        TB(0) = 0;
        ui_window_open(0x25);
    } else if (TB(0xAD4) == 2) {
        TB(0xAD4) = 0;
        TB(0) = 0;
        list_a_save_new();
    } else if (TB(0xADE) == 2) {
        TB(0xADE) = 0;
        TB(0) = 0;
        TT8(0xAC60) = TT8(0xAC60) == 0;
    } else if (TB(0xAF2) == 2) {
        TB(0xAF2) = 0;
        TB(0) = 0;
        TT8(0x44) = TT8(0x44) == 0;
    } else if (TB(0xAFC) == 2) {
        TB(0xAFC) = 0;
        TB(0) = 0;
        v = TT8(0x45) == 0;
        TT8(0x45) = v;
        TT8(0x47) = v;
    }
}
""")

we(0x40055978, 348, "system_calibration_button_handler",
   "Button 0x188 released: message 0x9C00 at the centre of window rectangle 0x19C, system_calibration, restores the area and shows the message for its result (-2: 0x9C00 + 0x4E0, -1: + 0x340, 0: + 0x680, 1: + 0x1A0; none otherwise), waits 1 s, closes window 0x1D and invalidates from window 5.", TBE + MSGE + """
void touch_button_update_0188(void);
int system_calibration(void);
void rect_42e68c38_draw(void);
void ui_window_invalidate_from(u32 id);   /* r1 is what ui_window_close left, as in the stock code */

void system_calibration_button_handler(void)
{
    u32 cx, cy, t, p;
    int rc;
    touch_button_update_0188();
    cx = (unsigned short)(WR16(0x19C) + (WR16(0x1A0) >> 1));
    cy = (unsigned short)(WR16(0x19E) + (WR16(0x1A2) >> 1));
    if (TB(0x188) != 2)
        return;
    TB(0x188) = 0;
    TB(0) = 0;
    MSG(cx, cy, 0x9C00);
    rc = system_calibration();
    rect_42e68c38_draw();
    switch (rc) {
    case -2: t = 0x4E0, p = 0x674; break;
    case -1: t = 0x340, p = 0x4D4; break;
    case 0: t = 0x680, p = 0x814; break;
    case 1: t = 0x1A0, p = 0x334; break;
    default: t = 0; break;
    }
    if (t)
        message_box_draw((int)cx + TX8(0x9C00 + p), (int)cy + TX8(0x9C00 + p + 1), TX32(0x9C00 + t), 0);
    delay_ms(1000);
    ui_window_close(0x1D);
    ui_window_invalidate_from(5);
}
""")

we(0x40056804, 368, "buttons_be2_handler",
   "Buttons 0xBE2, 0xBEC, 0xBF6, 0xC00 set word +0xF4 of block 0x42E68668 to 0..3; 0xC0A / 0xC14 toggle its bytes +1 / +2; 0xC1E opens window 0x2A. Ends by rewriting the touch byte.", TBE + """
void touch_group_0BE2(void);
#define MK8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MK32(o) (*(volatile u32 *)(0x42E68668u + (o)))

void buttons_be2_handler(void)
{
    u32 i;
    touch_group_0BE2();
    for (i = 0; i < 4; i++) {
        if (TB(0xBE2 + 10 * i) == 2) {
            TB(0xBE2 + 10 * i) = 0;
            MK32(0xF4) = i;
            TB(0) = 0;
            return;
        }
    }
    if (TB(0xC0A) == 2) {
        TB(0xC0A) = 0;
        MK8(1) = MK8(1) == 0;
        TB(0) = 0;
        return;
    }
    if (TB(0xC14) == 2) {
        TB(0xC14) = 0;
        MK8(2) = MK8(2) == 0;
        TB(0) = 0;
        return;
    }
    if (TB(0xC1E) == 2) {
        TB(0xC1E) = 0;
        TB(0) = 0;
        ui_window_open(0x2A);
    }
    TB(0) = TB(0);
}
""")

we(0x40056080, 396, "numpad_buttons_b06",
   "Buttons 0xB06..0xB2E open the numpad for field 1..5 (modes 4, 4, 1, 1, 5 at x = 0, 0xAB, 0x156, 0x201, 0x236, y = 0xE9) after clearing byte 0x42E68A8A, and set the numpad target: word 0x42E68AB8 = 0x42E18114 + 4k, byte 0x42E68AD4 = 0, word 0x42E68AD8 = 0, word 0x42E18110 = field. Button 0xB38 toggles byte 0x42E1810E. The four target words are always rewritten.", TBE + """
void touch_group_0B06(void);
void numpad_open_mode1(u32 x, u32 y);
void numpad_open_mode4(u32 x, u32 y);
void numpad_open_mode5(u32 x, u32 y);
#define TT8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))
#define TT32(o) (*(volatile u32 *)(0x42E180C8u + (o)))
#define NP8(o) (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define NP32(o) (*(volatile u32 *)(0x42E68A88u + (o)))

void numpad_buttons_b06(void)
{
    u32 ptr, b, w, fld;
    touch_group_0B06();
    if (TB(0xB06) == 2) {
        TB(0xB06) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode4(0, 0xE9);
        ptr = 0x42E180C8u + 0x4C, b = 0, fld = 1, w = 0;
    } else if (TB(0xB10) == 2) {
        TB(0xB10) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode4(0xAB, 0xE9);
        ptr = 0x42E180C8u + 0x50, b = 0, fld = 2, w = 0;
    } else {
        ptr = NP32(0x30);
        b = NP8(0x4C);
        w = NP32(0x50);
        fld = TT32(0x48);
        if (TB(0xB1A) == 2) {
            TB(0xB1A) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x156, 0xE9);
            ptr = 0x42E180C8u + 0x54, b = 0, fld = 3, w = 0;
        } else if (TB(0xB24) == 2) {
            TB(0xB24) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x201, 0xE9);
            ptr = 0x42E180C8u + 0x58, b = 0, fld = 4, w = 0;
        } else if (TB(0xB2E) == 2) {
            TB(0xB2E) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode5(0x236, 0xE9);
            ptr = 0x42E180C8u + 0x5C, b = 0, fld = 5, w = 0;
        } else if (TB(0xB38) == 2) {
            TB(0xB38) = 0;
            TB(0) = 0;
            TT8(0x46) = TT8(0x46) == 0;
        }
    }
    TT32(0x48) = fld;
    NP32(0x50) = w;
    NP8(0x4C) = b;
    NP32(0x30) = ptr;
}
""")

we(0x400623F4, 76, "levels_update_chain",
   "Unless touch byte +0x17 is set: when byte +4 of block 0x42E68668 is set runs 0x40061DB0 and 0x40061A70; then 0x4006053C, level_stats_5f010, level_stats_60008 and 0x4006233C.", """
void fft_levels_db(void);
void levels_samples_window(void);
void marker_values_compute(void);
void level_stats_5f010(void);
void level_stats_60008(void);
void level_stats_61000(void);

void levels_update_chain(void)
{
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (*(volatile unsigned char *)(0x42E68668u + 4)) {
        fft_levels_db();
        levels_samples_window();
    }
    marker_values_compute();
    level_stats_5f010();
    level_stats_60008();
    level_stats_61000();
}
""", src="trace")

we(0x400634F0, 224, "fpga_params_0e_13_write",
   "cursor_27e_clamp, then FPGA parameters 0x0E..0x13 for the channel chosen by SYS+0x27C (0: SYS+0xE0, SYS+0x60, slot SYS+0x1C4[SYS+0x41], words +0x18 / +0x38 of 0x42E53A0C; else SYS+0x1A0, SYS+0x120, slot [SYS+0x105], words +0x14 / +0x34), parameter 0x10 being the 2-byte SYS+0x27E; then smec_commit(0xB8) and fpga_cmd17_write.", """
void cursor_27e_clamp(void);
void smec_commit(u32 idx);
void fpga_cmd17_write(void);
#define PB32(o) (*(volatile u32 *)(0x42E53A0Cu + (o)))

void fpga_params_0e_13_write(void)
{
    u32 a, b, c, d, e, w;
    cursor_27e_clamp();
    if (SYS8(0x27C) == 0) {
        a = SYS32(0xE0);
        b = SYS32(0x60);
        d = PB32(0x18);
        e = PB32(0x38);
        c = *(volatile u32 *)(0x40544E68u + 0x1C4 + 4 * SYS8(0x41));
    } else {
        a = SYS32(0x1A0);
        b = SYS32(0x120);
        d = PB32(0x14);
        e = PB32(0x34);
        c = *(volatile u32 *)(0x40544E68u + 0x1C4 + 4 * SYS8(0x105));
    }
    fpga_param_updater(0xE, 4, a, a);
    fpga_param_updater(0xF, 4, b, b);
    w = SYS16(0x27E) | ((u32)SYS16(0x280) << 16);      /* the stock reads the word at SYS+0x27E unaligned */
    fpga_param_updater(0x10, 2, w, (u32)(int)(short)SYS16(0x27E));
    fpga_param_updater(0x11, 4, c, c);
    fpga_param_updater(0x12, 4, d, d);
    fpga_param_updater(0x13, 4, e, e);
    smec_commit(0xB8);
    fpga_cmd17_write();
}
""", src="acquisition")

we(0x40064C28, 200, "gen_selftest_64c28",
   "Generator self-test: four generator settings (words 0x42E42880 / 0x42E42884 of block 0x42E3F984 = 0x2D32355 / 1800, 0x2D32355 / 1800, 0x1797D51 / 1300, 0xBC84B1 / 1300; first also byte +0x2EFA = 0 and word 0x42E42888 = 0), each applied with gen_update and checked by 0x4006453C (0xB, 0, 1), (0x15, 1, 1), (0x16, 2, 1), (0x17, 3, 2); returns 1 when all four pass (bit 0 of the AND).", """
void gen_update(void);
int func_4006453c(u32 a, u32 b, u32 c);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))

int gen_selftest_64c28(void)
{
    u32 a, b, c;
    GB8(0x2EFA) = 0;
    GB32(0x2EFC) = 0x2D32355;
    GB32(0x2F04) = 0;
    GB32(0x2F00) = 1800;
    gen_update();
    a = func_4006453c(0xB, 0, 1);
    GB32(0x2EFC) = 0x2D32355;
    GB32(0x2F00) = 1800;
    gen_update();
    a &= func_4006453c(0x15, 1, 1);
    GB32(0x2EFC) = 0x1797D51;
    GB32(0x2F00) = 1300;
    gen_update();
    b = func_4006453c(0x16, 2, 1);
    GB32(0x2EFC) = 0xBC84B1;
    GB32(0x2F00) = 1300;
    gen_update();
    c = func_4006453c(0x17, 3, 2);
    return c & b & a & 1;
}
""", src="init")
