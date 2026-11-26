/*
 * trigger_level_drag_buttons -- stock 0x40054540 (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x71E pressed (state 1): clears it and the touch byte, touch_moved_flag and, in drag mode 0x12 (byte 0x42E68998), keeps s16 SYS+0x27E as the 64-bit start. Else when byte 0x71F is set: clears it, sets the touch byte, touch_drag_delta_b and in mode 0x12 sets SYS+0x27E = start + delta (word +0x34).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"
#include "state/touch_ctl.h"


#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))

void trigger_level_drag_buttons(void)
{
    trigger_level_drag_buttons_update();
    if (BREC(182, state) == 1) {
        BREC(182, state) = 0;
        BREC_BYTE0 = 0;
        touch_moved_flag();
        if (TCS(code) == 0x12)
            TCS(drag_start) = (short)SI(trigger_level);
    } else if (BREC(182, touch_code)) {
        BREC(182, touch_code) = 0;
        BREC_BYTE0 = 1;
        touch_drag_delta_b();
        if (TCS(code) == 0x12)
            SI(trigger_level) = (unsigned short)(TCS(drag_dy) + DG32(8));
    }
}
