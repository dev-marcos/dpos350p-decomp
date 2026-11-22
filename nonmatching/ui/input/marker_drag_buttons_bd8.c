/*
 * marker_drag_buttons_bd8 -- stock 0x40056724 (220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0xBD8 pressed: touch_pick_a and, in drag mode 0x10 / 0x11, keeps word +0x5C / +0x60 of block 0x42E68668 as the 64-bit drag start. Byte 0xBD9 = 1: touch_drag_delta_c and in those modes sets that word to start - word +0x30 of the drag block.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
#define DG8(o) (*(volatile unsigned char *)(0x42E68998u + (o)))
#define DG32(o) (*(volatile u32 *)(0x42E68998u + (o)))
#define DG64(o) (*(volatile long long *)(0x42E68998u + (o)))

void touch_button_update_0BD8(void);
void touch_pick_a(void);
void touch_drag_delta_c(void);
#define MK(o) (*(volatile int *)(0x42E68668u + (o)))

void marker_drag_buttons_bd8(void)
{
    u32 m;
    touch_button_update_0BD8();
    if (TB(0xBD8) == 1) {
        TB(0xBD8) = 0;
        TB(0) = 0;
        touch_pick_a();
        m = DG8(0);
        if (m == 0x10)
            DG64(8) = MK(0x5C);
        else if (m == 0x11)
            DG64(8) = MK(0x60);
    }
    if (TB(0xBD9) != 1)
        return;
    TB(0xBD9) = 0;
    TB(0) = 1;
    touch_drag_delta_c();
    m = DG8(0);
    if (m == 0x10)
        MK(0x5C) = DG32(8) - DG32(0x30);
    else if (m == 0x11)
        MK(0x60) = DG32(8) - DG32(0x30);
}
