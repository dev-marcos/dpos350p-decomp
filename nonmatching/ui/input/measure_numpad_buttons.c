/*
 * measure_numpad_buttons -- stock 0x40056530 (500 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Measurement window numpad fields, buttons 0xB92..0xBC4: each clears byte 0x42E68A8A and opens the numpad (mode 3 at x = 2 / 0x92, mode 1 at 0x124, 0x1B6, 0x248, 0x28B; y = 0xE9) with the callback 0x400617AC, flag 0x42E68AD4 = 1 and a target in block 0x42E68668: the doubles +0x148 / +0x140 (word 0x42E68AB0), or the s64 +0x120, +0x128, +0x130, +0x138 (word 0x42E68AB4, with mode +0x18C = 1..4). Button 0xBCE toggles byte +3. Except for the first two, the four target words are always rewritten.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/entry_pad.h"
#include "state/levels_block.h"
#include "state/touch_buttons.h"

void inv_left(u32 id) __asm__("ui_window_invalidate_from");

#define CB 0x400617ACu

void measure_numpad_buttons(void)
{
    u32 fl, cb, tgt, mode;
    touch_group_0B92();
    if (BREC(296, state) == 2) {
        BREC(296, state) = 0;
        BREC_BYTE0 = 0;
        NUMPAD(f_002) = 0;
        numpad_open_mode3(2, 0xE9);
        NUMPAD(target_0) = 0x42E68668u + 0x148;
        NUMPAD(f_050) = CB;
        NUMPAD(f_04c) = 1;
        return;
    }
    fl = NUMPAD(f_04c);
    cb = NUMPAD(f_050);
    if (BREC(297, state) == 2) {
        BREC(297, state) = 0;
        BREC_BYTE0 = 0;
        NUMPAD(f_002) = 0;
        numpad_open_mode3(0x92, 0xE9);
        NUMPAD(target_0) = 0x42E68668u + 0x140;
        NUMPAD(f_050) = CB;
        NUMPAD(f_04c) = 1;
        return;
    }
    if (BREC(298, state) == 2) {
        BREC(298, state) = 0;
        BREC_BYTE0 = 0;
        NUMPAD(f_002) = 0;
        numpad_open_mode1(0x124, 0xE9);
        tgt = 0x42E68668u + 0x120, fl = 1, cb = CB, mode = 1;
    } else {
        tgt = NUMPAD(target_1);
        mode = LEVELS(window_mode);
        if (BREC(299, state) == 2) {
            BREC(299, state) = 0;
            BREC_BYTE0 = 0;
            NUMPAD(f_002) = 0;
            numpad_open_mode1(0x1B6, 0xE9);
            tgt = 0x42E68668u + 0x128, fl = 1, cb = CB, mode = 2;
        } else if (BREC(300, state) == 2) {
            BREC(300, state) = 0;
            BREC_BYTE0 = 0;
            NUMPAD(f_002) = 0;
            numpad_open_mode1(0x248, 0xE9);
            tgt = 0x42E68668u + 0x130, fl = 1, cb = CB, mode = 3;
        } else if (BREC(301, state) == 2) {
            BREC(301, state) = 0;
            BREC_BYTE0 = 0;
            NUMPAD(f_002) = 0;
            numpad_open_mode1(0x28B, 0xE9);
            tgt = 0x42E68668u + 0x138, fl = 1, cb = CB, mode = 4;
        } else if (BREC(302, state) == 2) {
            BREC(302, state) = 0;
            BREC_BYTE0 = 0;
            LEVELS(move_slow) = LEVELS(move_slow) == 0;
        }
    }
    NUMPAD(target_1) = tgt;
    LEVELS(window_mode) = mode;
    NUMPAD(f_050) = cb;
    NUMPAD(f_04c) = fl;
}
