/*
 * marker_drag_buttons_bd8 -- stock 0x40056724 (220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0xBD8 pressed: touch_pick_a and, in drag mode 0x10 / 0x11, keeps word +0x5C / +0x60 of block 0x42E68668 as the 64-bit drag start. Byte 0xBD9 = 1: touch_drag_delta_c and in those modes sets that word to start - word +0x30 of the drag block.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/levels_block.h"
#include "state/touch_buttons.h"
#include "state/touch_ctl.h"

#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))


void marker_drag_buttons_bd8(void)
{
    u32 m;
    touch_button_update_0BD8();
    if (BREC(303, state) == 1) {
        BREC(303, state) = 0;
        BREC_BYTE0 = 0;
        touch_pick_a();
        m = TCS(code);
        if (m == 0x10)
            TCS(drag_start) = LEVELS(marker1_x);
        else if (m == 0x11)
            TCS(drag_start) = LEVELS(marker2_x);
    }
    if (BREC(303, touch_code) != 1)
        return;
    BREC(303, touch_code) = 0;
    BREC_BYTE0 = 1;
    touch_drag_delta_c();
    m = TCS(code);
    if (m == 0x10)
        LEVELS(marker1_x) = DG32(8) - TCS(drag_dx);
    else if (m == 0x11)
        LEVELS(marker2_x) = DG32(8) - TCS(drag_dx);
}
