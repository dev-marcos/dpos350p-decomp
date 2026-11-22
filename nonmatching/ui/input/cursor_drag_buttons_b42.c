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

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
#define DG8(o) (*(volatile unsigned char *)(0x42E68998u + (o)))
#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))
#define DG64(o) (*(volatile long long *)(0x42E68998u + (o)))

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
