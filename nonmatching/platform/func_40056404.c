/*
 * buttons_b56_handler -- stock 0x40056404 (292 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xB56 (restore SYS_INFO, reset and clear the screen, open windows 1 and 5, SYS+0x54E = 0, SYS+0x5F0 = 100000000), 0xB60 (open window 0x29), 0xB6A (save a screenshot), 0xB74 (toggle word +0x58 of block 0x42E68668), 0xB7E (measurement setup unless locked), 0xB88 (toggle byte +0 of that block).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_group_0B56(void);
void sys_info_restore(void);
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_a_save_new(void);
void func_40062e14_unless_locked(void);
#define MK(o) (*(volatile u32 *)(0x42E68668u + (o)))

void buttons_b56_handler(void)
{
    touch_group_0B56();
    if (TB(0xB56) == 2) {
        TB(0xB56) = 0;
        TB(0) = 0;
        sys_info_restore();
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        SYS8(0x54E) = 0;
        SYS32(0x5F0) = 100000000;
    } else if (TB(0xB60) == 2) {
        TB(0xB60) = 0;
        TB(0) = 0;
        ui_window_open(0x29);
    } else if (TB(0xB6A) == 2) {
        TB(0xB6A) = 0;
        TB(0) = 0;
        list_a_save_new();
    } else if (TB(0xB74) == 2) {
        TB(0xB74) = 0;
        TB(0) = 0;
        MK(0x58) = MK(0x58) == 0;
    } else if (TB(0xB7E) == 2) {
        TB(0xB7E) = 0;
        TB(0) = 0;
        func_40062e14_unless_locked();
    } else if (TB(0xB88) == 2) {
        TB(0xB88) = 0;
        TB(0) = 0;
        *(volatile unsigned char *)0x42E68668u = *(volatile unsigned char *)0x42E68668u == 0;
    }
}
