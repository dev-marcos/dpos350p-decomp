/*
 * gen_panel_buttons -- stock 0x40053B78 (2504 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch handler of the signal generator panel (block GEN = 0x42E3F984). After touch_buttons_6xx_update, the first record of 0x42E61FF8 in state 2 (released) is reset to 0 and handled: + 0x624 + 0xA x k (k = 0 .. 3) selects field k (byte GEN+1 = k, word GEN+4 = GEN + 8 + k, the address of the field's digit position); + 0x656 / + 0x660 add / subtract one unit of the selected digit to the field value (words GEN+0x2EFC + 4 x field; digit steps 10^7 .. 1 for field 0, 1000 .. 1 for fields 1 and 2, 100 .. 1 for field 3) and then call gen_update and ui_window_invalidate_from(3); + 0x66A / + 0x674 move the digit position left / right (limits 7, 3, 3, 2); + 0x67E opens the number pad (numpad_open_mode1 for field 0, mode4 for 1 and 2, mode6 for 3, at 0x1BD, 0xA8) with the field value as target and gen_update as callback; + 0x64C toggles byte GEN+0xC; + 0x688 + 0xA x k (k = 0 .. 14) selects waveform k (byte GEN+0x2EFA), clears GEN+0xC and applies it. The flag byte 0x42E61FF8 is cleared too, except for the digit-right button.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "ui/input.h"
#include "ui/windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_invalidate_from_raw(u32 n) __asm__("ui_window_invalidate_from");

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records; byte 0 is the pending-touch flag */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define GEN 0x42E3F984u
#define PAD 0x42E68A88u              /* number pad state */

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
    ui_window_invalidate_from_raw(3);
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
            ui_window_invalidate_from_raw(3);
            return;
        }
    }
}
