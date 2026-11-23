/*
 * numpad_key -- stock 0x4005FE94 (228 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Numpad key k: 1 toggles the sign byte 0x42E68AA1 (modes 2..4 of byte 0x42E68A89 only) and applies it; 2 appends '.' once (byte 0x42E68AA0); 3..12 append the digits '0'..'9' while the length byte 0x42E68A9F is below 10; 13 is backspace; 14..16 run 0x4005FC84 (unit = k) and close the numpad (numpad_close gets the registers 0x4005FC84 left).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "state/entry_pad.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void numpad_close_raw(void) __asm__("numpad_close");


void numpad_key(u32 k)
{
    if (k == 1) {
        u32 m = NUMPAD(mode);
        if (m == 2 || m == 3 || m == 4) {
            NUMPAD(has_minus) = NUMPAD(has_minus) == 0;
            numpad_sign_apply();
        }
    } else if (k == 2) {
        if (NUMPAD(has_dot) == 0) {
            NUMPAD(has_dot) = 1;
            numpad_append_char(0x2E);
        }
    } else if (k == 0xD) {
        numpad_backspace();
    } else if (k - 3 < 10) {
        if (NUMPAD(len) < 10)
            numpad_append_char((u32)(int)(signed char)(k + 0x2D));
    } else if (k == 0xE || k == 0xF || k == 0x10) {
        numpad_value_parse(k);
        numpad_close_raw();
    }
}
