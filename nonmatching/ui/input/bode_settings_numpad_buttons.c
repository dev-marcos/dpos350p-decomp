/*
 * bode_settings_numpad_buttons -- stock 0x40056080 (396 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xB06..0xB2E open the numpad for field 1..5 (modes 4, 4, 1, 1, 5 at x = 0, 0xAB, 0x156, 0x201, 0x236, y = 0xE9) after clearing byte 0x42E68A8A, and set the numpad target: word 0x42E68AB8 = 0x42E18114 + 4k, byte 0x42E68AD4 = 0, word 0x42E68AD8 = 0, word 0x42E18110 = field. Button 0xB38 toggles byte 0x42E1810E. The four target words are always rewritten.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/bode_block.h"
#include "state/entry_pad.h"
#include "state/touch_buttons.h"

void bode_settings_numpad_buttons(void)
{
    u32 ptr, b, w, fld;
    bode_settings_numpad_buttons_update();
    if (BREC(282, state) == 2) {
        BREC(282, state) = 0;
        BREC_BYTE0 = 0;
        NUMPAD(f_002) = 0;
        numpad_open_mode4(0, 0xE9);
        ptr = 0x42E180C8u + 0x4C, b = 0, fld = 1, w = 0;
    } else if (BREC(283, state) == 2) {
        BREC(283, state) = 0;
        BREC_BYTE0 = 0;
        NUMPAD(f_002) = 0;
        numpad_open_mode4(0xAB, 0xE9);
        ptr = 0x42E180C8u + 0x50, b = 0, fld = 2, w = 0;
    } else {
        ptr = NUMPAD(target_2);
        b = NUMPAD(f_04c);
        w = NUMPAD(f_050);
        fld = BODES(f_048);
        if (BREC(284, state) == 2) {
            BREC(284, state) = 0;
            BREC_BYTE0 = 0;
            NUMPAD(f_002) = 0;
            numpad_open_mode1(0x156, 0xE9);
            ptr = 0x42E180C8u + 0x54, b = 0, fld = 3, w = 0;
        } else if (BREC(285, state) == 2) {
            BREC(285, state) = 0;
            BREC_BYTE0 = 0;
            NUMPAD(f_002) = 0;
            numpad_open_mode1(0x201, 0xE9);
            ptr = 0x42E180C8u + 0x58, b = 0, fld = 4, w = 0;
        } else if (BREC(286, state) == 2) {
            BREC(286, state) = 0;
            BREC_BYTE0 = 0;
            NUMPAD(f_002) = 0;
            numpad_open_mode5(0x236, 0xE9);
            ptr = 0x42E180C8u + 0x5C, b = 0, fld = 5, w = 0;
        } else if (BREC(287, state) == 2) {
            BREC(287, state) = 0;
            BREC_BYTE0 = 0;
            BODES(move_slow) = BODES(move_slow) == 0;
        }
    }
    BODES(f_048) = fld;
    NUMPAD(f_050) = w;
    NUMPAD(f_04c) = b;
    NUMPAD(target_2) = ptr;
}
