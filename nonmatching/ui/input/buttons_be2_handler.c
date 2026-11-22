/*
 * buttons_be2_handler -- stock 0x40056804 (368 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xBE2, 0xBEC, 0xBF6, 0xC00 set word +0xF4 of block 0x42E68668 to 0..3; 0xC0A / 0xC14 toggle its bytes +1 / +2; 0xC1E opens window 0x2A. Ends by rewriting the touch byte.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "ui/windows.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

#define MK8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MK32(o) (*(volatile u32 *)(0x42E68668u + (o)))

void buttons_be2_handler(void)
{
    u32 i;
    touch_group_0BE2();
    for (i = 0; i < 4; i++) {
        if (TB(0xBE2 + 10 * i) == 2) {
            TB(0xBE2 + 10 * i) = 0;
            MK32(0xF4) = i;
            TB(0) = 0;
            return;
        }
    }
    if (TB(0xC0A) == 2) {
        TB(0xC0A) = 0;
        MK8(1) = MK8(1) == 0;
        TB(0) = 0;
        return;
    }
    if (TB(0xC14) == 2) {
        TB(0xC14) = 0;
        MK8(2) = MK8(2) == 0;
        TB(0) = 0;
        return;
    }
    if (TB(0xC1E) == 2) {
        TB(0xC1E) = 0;
        TB(0) = 0;
        ui_window_open(0x2A);
    }
    TB(0) = TB(0);
}
