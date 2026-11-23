/*
 * cursor_drag_buttons_b42 -- stock 0x40056214 (220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0xB42 pressed (state 1): touch_pick_b and, in drag mode 0xE / 0xF, keeps cursor word 0x42E22D2C / +4 as the 64-bit drag start. Byte 0xB43 = 1: touch_drag_delta_d and in those modes sets the cursor to start - word +0x30 of the drag block, then cursor_ac64_clamp.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"
#include "state/touch_ctl.h"

#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))

#define CUR(o) (*(volatile int *)(0x42E22D2Cu + (o)))

void cursor_drag_buttons_b42(void)
{
    u32 m;
    touch_button_update_0B42();
    if (BREC(288, state) == 1) {
        BREC(288, state) = 0;
        BREC_BYTE0 = 0;
        touch_pick_b();
        m = TCS(code);
        if (m == 0xE)
            TCS(drag_start) = CUR(0);
        else if (m == 0xF)
            TCS(drag_start) = CUR(4);
    }
    if (BREC(288, touch_code) != 1)
        return;
    BREC(288, touch_code) = 0;
    BREC_BYTE0 = 1;
    touch_drag_delta_d();
    m = TCS(code);
    if (m == 0xE)
        CUR(0) = DG32(8) - TCS(drag_dx);
    else if (m == 0xF)
        CUR(4) = DG32(8) - TCS(drag_dx);
    else
        return;
    cursor_ac64_clamp();
}
