/*
 * button_71e_drag_handler -- stock 0x40054540 (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x71E pressed (state 1): clears it and the touch byte, touch_moved_flag and, in drag mode 0x12 (byte 0x42E68998), keeps s16 SYS+0x27E as the 64-bit start. Else when byte 0x71F is set: clears it, sets the touch byte, touch_drag_delta_b and in mode 0x12 sets SYS+0x27E = start + delta (word +0x34).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

#define DG8(o) (*(volatile unsigned char *)(0x42E68998u + (o)))
#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))
#define DG64(o) (*(volatile long long *)(0x42E68998u + (o)))

void button_71e_drag_handler(void)
{
    touch_button_update_071E();
    if (TB(0x71E) == 1) {
        TB(0x71E) = 0;
        TB(0) = 0;
        touch_moved_flag();
        if (DG8(0) == 0x12)
            DG64(8) = (short)SYS16(0x27E);
    } else if (TB(0x71F)) {
        TB(0x71F) = 0;
        TB(0) = 1;
        touch_drag_delta_b();
        if (DG8(0) == 0x12)
            SYS16(0x27E) = (unsigned short)(DG32(0x34) + DG32(8));
    }
}
