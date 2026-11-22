/*
 * main_menu_buttons -- stock 0x40052100 (384 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x2, 0xC, 0x16, 0x20 open windows 6, 0xA, 0xB, 0xC; 0x2A / 0x34 toggle SYS+0x802 / +0x803; 0x48 is only cleared; 0x3E saves a waveform (list_b_save_new); 0x52 resets the zoom and toggles SYS+0x2AA when the zoom count SYS+0x588 is above 12.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_open(u32 id);

void touch_group_0002(void);
void list_b_save_new(void);
void zoom_reset(void);

void main_menu_buttons(void)
{
    u32 i;
    touch_group_0002();
    for (i = 0; i < 4; i++) {
        if (TB(2 + 10 * i) == 2) {
            TB(2 + 10 * i) = 0;
            TB(0) = 0;
            ui_window_open(i == 0 ? 6 : 9 + i);
            return;
        }
    }
    if (TB(0x2A) == 2) {
        TB(0x2A) = 0;
        TB(0) = 0;
        SYS8(0x802) = SYS8(0x802) == 0;
    } else if (TB(0x34) == 2) {
        TB(0x34) = 0;
        TB(0) = 0;
        SYS8(0x803) = SYS8(0x803) == 0;
    } else if (TB(0x48) == 2) {
        TB(0x48) = 0;
        TB(0) = 0;
    } else if (TB(0x3E) == 2) {
        TB(0x3E) = 0;
        TB(0) = 0;
        list_b_save_new();
    } else if (TB(0x52) == 2) {
        TB(0x52) = 0;
        TB(0) = 0;
        if (SYS8(0x588) > 12) {
            zoom_reset();
            SYS8(0x2AA) = SYS8(0x2AA) == 0;
        }
    }
}
