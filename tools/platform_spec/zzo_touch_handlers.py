# Touch handler of the signal generator panel and the drag / tap handler of the plot area.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records; byte 0 is the pending-touch flag */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))
"""

wr(0x40053B78, 2504, "gen_panel_buttons",
   "Touch handler of the signal generator panel (block GEN = 0x42E3F984). After touch_buttons_6xx_update, the first record of 0x42E61FF8 in state 2 (released) is reset "
   "to 0 and handled: + 0x624 + 0xA x k (k = 0 .. 3) selects field k (byte GEN+1 = k, word GEN+4 = GEN + 8 + k, the address of the field's digit position); + 0x656 / "
   "+ 0x660 add / subtract one unit of the selected digit to the field value (words GEN+0x2EFC + 4 x field; digit steps 10^7 .. 1 for field 0, 1000 .. 1 for fields 1 "
   "and 2, 100 .. 1 for field 3) and then call gen_update and ui_window_invalidate_from(3); + 0x66A / + 0x674 move the digit position left / right (limits 7, 3, 3, 2); "
   "+ 0x67E opens the number pad (numpad_open_mode1 for field 0, mode4 for 1 and 2, mode6 for 3, at 0x1BD, 0xA8) with the field value as target and gen_update as "
   "callback; + 0x64C toggles byte GEN+0xC; + 0x688 + 0xA x k (k = 0 .. 14) selects waveform k (byte GEN+0x2EFA), clears GEN+0xC and applies it. The flag byte "
   "0x42E61FF8 is cleared too, except for the digit-right button.",
   COM + """
#define GEN 0x42E3F984u
#define PAD 0x42E68A88u              /* number pad state */

void touch_buttons_6xx_update(void);
void gen_update(void);
void ui_window_invalidate_from(u32 n);
void numpad_open_mode1(u32 x, u32 y);
void numpad_open_mode4(u32 x, u32 y);
void numpad_open_mode6(u32 x, u32 y);

static const int step8[8] = { 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };
static const int step4[4] = { 1000, 100, 10, 1 };
static const int step3[3] = { 100, 10, 1 };
static const unsigned char pos_max[4] = { 7, 3, 3, 2 };

static int take(u32 rec)
{
    if (B8(BTN + rec) != 2)
        return 0;
    B8(BTN + rec) = 0;
    return 1;
}

static void step(int dir)
{
    u32 fld = B8(GEN + 1), p;

    if (fld == 0) {
        p = B8(GEN + 8);
        if (p < 8)
            W32(GEN + 0x2EFC) += dir * step8[p];
    } else if (fld == 1 || fld == 2) {
        p = B8(GEN + 8 + fld);
        if (p < 4)
            W32(GEN + 0x2EFC + 4 * fld) += dir * step4[p];
    } else if (fld == 3) {
        p = B8(GEN + 0xB);
        if (p < 3)
            W32(GEN + 0x2F08) += dir * step3[p];
    }
    B8(BTN) = 0;
    gen_update();
    ui_window_invalidate_from(3);
}

void gen_panel_buttons(void)
{
    u32 fld, ptr;
    int k;

    touch_buttons_6xx_update();
    for (k = 0; k < 4; k++) {
        if (take(0x624 + 0xA * k)) {
            B8(BTN) = 0;
            B8(GEN + 1) = k;
            W32(GEN + 4) = GEN + 8 + k;
            return;
        }
    }
    if (take(0x656)) {
        step(1);
        return;
    }
    if (take(0x660)) {
        step(-1);
        return;
    }
    if (take(0x66A)) {
        B8(BTN) = 0;
        fld = B8(GEN + 1);
        if (fld < 4 && B8(GEN + 8 + fld) != 0)
            B8(GEN + 8 + fld) = B8(GEN + 8 + fld) - 1;
        return;
    }
    if (take(0x674)) {
        fld = B8(GEN + 1);
        if (fld < 4 && B8(GEN + 8 + fld) < pos_max[fld])
            B8(GEN + 8 + fld) = B8(GEN + 8 + fld) + 1;
        return;
    }
    if (take(0x67E)) {
        B8(BTN) = 0;
        fld = B8(GEN + 1);
        if (fld > 3)
            return;
        B8(PAD + 2) = 0;
        if (fld == 0)
            numpad_open_mode1(0x1BD, 0xA8);
        else if (fld == 3)
            numpad_open_mode6(0x1BD, 0xA8);
        else
            numpad_open_mode4(0x1BD, 0xA8);
        ptr = GEN + 0x2EFC + 4 * fld;
        W32(PAD + 0x30) = ptr;
        B8(PAD + 0x4C) = 1;
        W32(PAD + 0x50) = 0x4004D18Cu;          /* gen_update */
        return;
    }
    if (take(0x64C)) {
        B8(BTN) = 0;
        B8(GEN + 0xC) = B8(GEN + 0xC) == 0;
        return;
    }
    for (k = 0; k < 15; k++) {
        if (take(0x688 + 0xA * k)) {
            B8(BTN) = 0;
            B8(GEN + 0x2EFA) = k;
            B8(GEN + 0xC) = 0;
            gen_update();
            ui_window_invalidate_from(3);
            return;
        }
    }
}
""")

wr(0x400545F8, 1664, "plot_touch_drag",
   "Drag and tap handler of the plot area (record 0x42E61FF8 + 0xF2 / + 0xF3, drag state U = 0x42E68998). Press (record + 0xF2 = 1): ui_window_invalidate_from(3), "
   "func_40069b0c picks the dragged object (byte U), whose start values are saved as int64 at U+8 / +0x10 / +0x18: 1 trigger position (SYS+0x570, or the zoom one "
   "SYS+0x578 in split view), 2 zoom position, 3 / 4 position plus CH1 / CH2 offset (SYS+0x60 / +0x120) plus the trigger level SYS+0x27E when that channel is the "
   "trigger channel, 5 / 6 zoom position plus CH1 / CH2 offset, 7 .. 10 cursor words SYS+0x808 .. +0x814, 11 / 12 words SYS+0x3DA534 / +0x3DA538, 13 both offsets; the "
   "press time 0x42E68CD8 is copied to 0x42E68C78. Move (record + 0xF3 = 1, flag byte set to 1): touch_drag_delta gives dx / dy (words U+0x30 / U+0x34) and the object "
   "is moved: positions by dx x the pixel time (int64 SYS+0x590, zoom SYS+0x5A0; only when SYS+0x580 is set for 1 .. 4), offsets and trigger level by dy, cursors by "
   "dx or dy with their clamp functions. Release (record + 0xF2 = 2): a tap (start and end point equal, under 600 ticks) on the right half of the plot steps the time "
   "base down (zoom_index_sub in split view), on the left half up (zoom_index_add), then clears SYS+0x586, acquisition_apply and ui_window_invalidate_from(3).",
   COM + """
#define U 0x42E68998u                /* drag state */
#define TCH 0x42E68BB0u              /* touch point: +4 / +6 now, +8 / +0xA at press */
#define S64(a) (*(volatile long long *)(a))
#define SPLIT B8(SYS + 0x2AA)

void ui_window_invalidate_from(u32 n);
void func_40069b0c(void);
void touch_drag_delta(void);
void cursor_808_clamp(void);
void cursor_810_clamp(void);
void cursor_3da534_clamp(u32 n);
void timebase_index_step(u32 n);
void timebase_index_step_down(u32 n);
void zoom_index_add(u32 n);
void zoom_index_sub(u32 n);
void acquisition_apply(void);

static void press_save(void)
{
    u32 m = B8(U);

    switch (m) {
    case 1:
        S64(U + 8) = S64(SYS + (SPLIT ? 0x578 : 0x570));
        break;
    case 2:
        S64(U + 8) = S64(SYS + 0x578);
        break;
    case 3:
    case 4:
        S64(U + 8) = S64(SYS + (SPLIT ? 0x578 : 0x570));
        S64(U + 0x10) = (int)W32(SYS + (m == 3 ? 0x60 : 0x120));
        if (B8(SYS + 0x27C) == m - 3)
            S64(U + 0x18) = S16(SYS + 0x27E);
        break;
    case 5:
    case 6:
        S64(U + 8) = S64(SYS + 0x578);
        S64(U + 0x10) = (int)W32(SYS + (m == 5 ? 0x60 : 0x120));
        break;
    case 7:
    case 8:
    case 9:
    case 10:
        S64(U + 8) = (int)W32(SYS + 0x808 + 4 * (m - 7));
        break;
    case 11:
    case 12:
        S64(U + 8) = (int)W32(SYS + 0x3DA534 + 4 * (m - 11));
        break;
    case 13:
        S64(U + 8) = (int)W32(SYS + 0x60);
        S64(U + 0x10) = (int)W32(SYS + 0x120);
        break;
    }
}

static void h_drag(int dx)
{
    long long d = (long long)dx * S64(SYS + 0x590);

    if (SPLIT)
        S64(SYS + 0x578) = S64(U + 8) + d;
    else
        S64(SYS + 0x570) = S64(U + 8) - d;
}

static void move(void)
{
    u32 m = B8(U), lo8 = (u32)S64(U + 8);
    int dx = (int)W32(U + 0x30), dy = (int)W32(U + 0x34);

    switch (m) {
    case 1:
        if (B8(SYS + 0x580))
            h_drag(dx);
        break;
    case 2:
        if (B8(SYS + 0x580))
            S64(SYS + 0x578) = S64(U + 8) - (long long)dx * S64(SYS + 0x5A0);
        break;
    case 3:
    case 4:
        if (B8(SYS + 0x580))
            h_drag(dx);
        W32(SYS + (m == 3 ? 0x60 : 0x120)) = dy + (u32)S64(U + 0x10);
        if (B8(SYS + 0x27C) == m - 3)
            H16(SYS + 0x27E) = dy + (u32)S64(U + 0x18);
        break;
    case 5:
    case 6:
        S64(SYS + 0x578) = S64(U + 8) - (long long)dx * S64(SYS + 0x5A0);
        W32(SYS + (m == 5 ? 0x60 : 0x120)) = dy + (u32)S64(U + 0x10);
        break;
    case 7:
    case 8:
        W32(SYS + 0x808 + 4 * (m - 7)) = lo8 - dx;
        cursor_808_clamp();
        break;
    case 9:
    case 10:
        W32(SYS + 0x810 + 4 * (m - 9)) = lo8 - dy;
        cursor_810_clamp();
        break;
    case 11:
    case 12:
        W32(SYS + 0x3DA534 + 4 * (m - 11)) = lo8 - dx;
        cursor_3da534_clamp(m - 11);
        break;
    case 13:
        W32(SYS + 0x60) = lo8 - dx;
        W32(SYS + 0x120) = dy + (u32)S64(U + 0x10);
        break;
    }
}

void plot_touch_drag(void)
{
    int x;

    if (B8(BTN + 0xF2) == 1) {
        B8(BTN + 0xF2) = 0;
        B8(BTN) = 0;
        ui_window_invalidate_from(3);
        func_40069b0c();
        press_save();
        W32(0x42E68C78u) = W32(0x42E68CD8u);
    }
    if (B8(BTN + 0xF3) == 1) {
        B8(BTN + 0xF3) = 0;
        B8(BTN) = 1;
        touch_drag_delta();
        move();
    }
    if (B8(BTN + 0xF2) != 2)
        return;
    B8(BTN + 0xF2) = 0;
    B8(BTN) = 0;
    x = S16(TCH + 8);
    if (S16(TCH + 4) != x || S16(TCH + 6) != S16(TCH + 0xA) || W32(0x42E68CD8u) - W32(0x42E68C78u) >= 600)
        return;
    if (x >= (int)(H16(SYS + 4) + (H16(SYS + 8) >> 1))) {
        if (!SPLIT)
            timebase_index_step_down(1);
        else
            zoom_index_sub(1);
    } else {
        if (!SPLIT)
            timebase_index_step(1);
        else
            zoom_index_add(1);
    }
    H16(SYS + 0x586) = 0;
    acquisition_apply();
    ui_window_invalidate_from(3);
}
""")
