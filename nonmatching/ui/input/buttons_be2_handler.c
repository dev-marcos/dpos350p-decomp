/*
 * buttons_be2_handler -- stock 0x40056804 (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xBE2, 0xBEC, 0xBF6, 0xC00 set word +0xF4 of block 0x42E68668 to 0..3; 0xC0A / 0xC14 toggle its bytes +1 / +2; 0xC1E opens window 0x2A. Ends by rewriting the touch byte.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/levels_block.h"
#include "state/touch_buttons.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))


void buttons_be2_handler(void)
{
    u32 i;
    touch_group_0BE2();
    for (i = 0; i < 4; i++) {
        if (TB(0xBE2 + 10 * i) == 2) {
            TB(0xBE2 + 10 * i) = 0;
            LEVELS(spur_mode) = i;
            BREC_BYTE0 = 0;
            return;
        }
    }
    if (BREC(308, state) == 2) {
        BREC(308, state) = 0;
        LEVELS(panel_a_on) = LEVELS(panel_a_on) == 0;
        BREC_BYTE0 = 0;
        return;
    }
    if (BREC(309, state) == 2) {
        BREC(309, state) = 0;
        LEVELS(panel_b_on) = LEVELS(panel_b_on) == 0;
        BREC_BYTE0 = 0;
        return;
    }
    if (BREC(310, state) == 2) {
        BREC(310, state) = 0;
        BREC_BYTE0 = 0;
        ui_window_open(0x2A);
    }
    BREC_BYTE0 = BREC_BYTE0;
}
