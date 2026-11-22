/*
 * output_select_buttons -- stock 0x400537C4 (380 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x55C + 10i (i = 0..8): the first one released sets byte SYS+0x660 = i, applies the outputs (outputs_apply) and invalidates from window 3 (r1 is what outputs_apply left).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_open(u32 id);

void touch_group_055C(void);
void outputs_apply(void);
void ui_window_invalidate_from(u32 id);

void output_select_buttons(void)
{
    u32 i;
    touch_group_055C();
    for (i = 0; i < 9; i++) {
        if (TB(0x55C + 10 * i) == 2) {
            TB(0x55C + 10 * i) = 0;
            TB(0) = 0;
            SYS8(0x660) = i;
            outputs_apply();
            ui_window_invalidate_from(3);
            return;
        }
    }
}
