/*
 * output_select_buttons -- stock 0x400537C4 (380 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x55C + 10i (i = 0..8): the first one released sets byte SYS+0x660 = i, applies the outputs (outputs_apply) and invalidates from window 3 (r1 is what outputs_apply left).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_invalidate_from_raw(u32 id) __asm__("ui_window_invalidate_from");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

void output_select_buttons(void)
{
    u32 i;
    output_select_buttons_update();
    for (i = 0; i < 9; i++) {
        if (TB(0x55C + 10 * i) == 2) {
            TB(0x55C + 10 * i) = 0;
            BREC_BYTE0 = 0;
            SI(hires_option) = i;
            outputs_apply();
            ui_window_invalidate_from_raw(3);
            return;
        }
    }
}
