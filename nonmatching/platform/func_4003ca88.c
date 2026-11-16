/*
 * ui_window_redraw_dirty -- stock 0x4003CA88 (644 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, walks the 20 window-stack slots bottom to top and redraws each window (ids 1..42) that is dirty (+1), or that is not self-contained (+2 = 0) while some window below it, or itself, has +4 set; a window with +2 set is redrawn for its own +4 or +1. Clears +1 and +3 before the redraw and passes the old +3 as the animate flag.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
#define FB_W     1022u
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_back  ((unsigned short *)0x41BFBF68u)
#define fb_off   ((unsigned short *)0x4233CBBCu)
void ui_window_01_redraw(void);
void ui_window_02_redraw(void);
void ui_window_03_redraw(void);
void ui_window_04_redraw(void);
void ui_window_05_redraw(void);
void ui_window_06_redraw(u32 anim);
void ui_window_07_redraw(u32 anim);
void ui_window_08_redraw(u32 anim);
void ui_window_09_redraw(u32 anim);
void ui_window_10_redraw(u32 anim);
void ui_window_11_redraw(u32 anim);
void ui_window_12_redraw(u32 anim);
void ui_window_13_redraw(u32 anim);
void ui_window_14_redraw(u32 anim);
void ui_window_15_redraw(u32 anim);
void ui_window_16_redraw(u32 anim);
void ui_window_17_redraw(u32 anim);
void ui_window_18_redraw(u32 anim);
void ui_window_19_redraw(void);
void ui_window_20_redraw(u32 anim);
void ui_window_21_redraw(u32 anim);
void ui_window_22_redraw(u32 anim);
void ui_window_23_redraw(void);
void ui_window_24_redraw(u32 anim);
void ui_window_25_redraw(u32 anim);
void ui_window_26_redraw(u32 anim);
void ui_window_27_redraw(u32 anim);
void ui_window_28_redraw(void);
void ui_window_29_redraw(u32 anim);
void ui_window_30_redraw(u32 anim);
void ui_window_31_redraw(u32 anim);
void ui_window_32_redraw(u32 anim);
void ui_window_33_redraw(u32 anim);
void ui_window_34_redraw(void);
void ui_window_35_redraw(void);
void ui_window_36_redraw(void);
void ui_window_37_redraw(u32 anim);
void ui_window_38_redraw(void);
void ui_window_39_redraw(void);
void ui_window_40_redraw(void);
void ui_window_41_redraw(u32 anim);
void ui_window_42_redraw(u32 anim);

void ui_window_redraw_dirty(void)
{
    u32 acc = 0, slot;
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    for (slot = 0; slot < 20; slot++) {
        u32 id = WSTK[2 + slot];
        volatile unsigned char *w = WIN(id);
        u32 own = w[4], d = w[1], anim, need;
        acc |= own;
        need = (w[2] == 0 ? acc : own) | d;
        anim = w[3];
        if ((need & 0xFF) == 0)
            continue;
        w[1] = 0;
        w[3] = 0;
        switch (id) {
        case 1: ui_window_01_redraw(); break;
        case 2: ui_window_02_redraw(); break;
        case 3: ui_window_03_redraw(); break;
        case 4: ui_window_04_redraw(); break;
        case 5: ui_window_05_redraw(); break;
        case 6: ui_window_06_redraw(anim); break;
        case 7: ui_window_07_redraw(anim); break;
        case 8: ui_window_08_redraw(anim); break;
        case 9: ui_window_09_redraw(anim); break;
        case 10: ui_window_10_redraw(anim); break;
        case 11: ui_window_11_redraw(anim); break;
        case 12: ui_window_12_redraw(anim); break;
        case 13: ui_window_13_redraw(anim); break;
        case 14: ui_window_14_redraw(anim); break;
        case 15: ui_window_15_redraw(anim); break;
        case 16: ui_window_16_redraw(anim); break;
        case 17: ui_window_17_redraw(anim); break;
        case 18: ui_window_18_redraw(anim); break;
        case 19: ui_window_19_redraw(); break;
        case 20: ui_window_20_redraw(anim); break;
        case 21: ui_window_21_redraw(anim); break;
        case 22: ui_window_22_redraw(anim); break;
        case 23: ui_window_23_redraw(); break;
        case 24: ui_window_24_redraw(anim); break;
        case 25: ui_window_25_redraw(anim); break;
        case 26: ui_window_26_redraw(anim); break;
        case 27: ui_window_27_redraw(anim); break;
        case 28: ui_window_28_redraw(); break;
        case 29: ui_window_29_redraw(anim); break;
        case 30: ui_window_30_redraw(anim); break;
        case 31: ui_window_31_redraw(anim); break;
        case 32: ui_window_32_redraw(anim); break;
        case 33: ui_window_33_redraw(anim); break;
        case 34: ui_window_34_redraw(); break;
        case 35: ui_window_35_redraw(); break;
        case 36: ui_window_36_redraw(); break;
        case 37: ui_window_37_redraw(anim); break;
        case 38: ui_window_38_redraw(); break;
        case 39: ui_window_39_redraw(); break;
        case 40: ui_window_40_redraw(); break;
        case 41: ui_window_41_redraw(anim); break;
        case 42: ui_window_42_redraw(anim); break;
        default: break;
        }
    }
}
