/*
 * numpad_key_buttons -- stock 0x40055CC4 (616 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Numpad window, buttons 0xA20..0xAB6 (step 10): each sends its key code to numpad_key: 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 1, 2, 0xD, 0xE, 0xF, 0x10. The first (key 3) also invalidates from window 0x21.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void numpad_key_buttons(void)
{
    u32 i, k;
    touch_buttons_a20_update();
    if (BREC(259, state) == 2) {
        BREC(259, state) = 0;
        BREC_BYTE0 = 0;
        numpad_key(3);
        inv_left(0x21);
        return;
    }
    for (i = 1; i < 16; i++) {
        if (TB(0xA20 + 10 * i) == 2) {
            TB(0xA20 + 10 * i) = 0;
            BREC_BYTE0 = 0;
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
