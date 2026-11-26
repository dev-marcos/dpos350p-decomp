/*
 * list_b_window_buttons -- stock 0x40055114 (560 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * List B window. Entry buttons 0x840 + 10i (i below the page size SYS+0xF9A): select entry page * size + i of SYS+0xFA4; when it exists (below SYS+0xFA0 + 1) it toggles its mark (SYS+0x5DC6 + entry) in mark mode SYS+0xF90, else opens it full screen. 0x8D6 back to the main screens; 0x8E0 toggles mark mode (marks cleared); 0x8EA marks the page or leaves mark mode; 0x8F4 opens the delete window 0x19 when marking; 0x8FE / 0x908 page -/+. Most paths end by invalidating from window 0x18.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"
#include "state/touch_buttons.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

#define P16(o) (*(volatile unsigned short *)(0x40544E68u + 0xF90 + (o)))
#define PS16(o) (*(volatile short *)(0x40544E68u + 0xF90 + (o)))

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void list_b_window_buttons(void)
{
    u32 i;
    int nv;
    touch_buttons_840_update();
    if (P16(0xA)) {
        i = 0;
        do {
            if (TB(0x840 + 10 * i) == 2) {
                int ps, tot, c;
                TB(0x840 + 10 * i) = 0;
                BREC_BYTE0 = 0;
                ps = P16(0xA);
                tot = PS16(0x10);
                PS16(0x14) = (short)(ps * sdv(PS16(0x14), ps) + (int)i);
                c = PS16(0x14);
                if (tot >= c + 1) {
                    if (SI(list_mark_mode)) {
                        volatile unsigned char *m = (volatile unsigned char *)(0x40544E68u + 0x5DC6 + c);
                        *m = *m == 0;
                    } else {
                        ui_windows_reset();
                        list_b_current_load();
                        fb_clear_current();
                        ui_windows_open_1_5();
                    }
                }
            }
            i = (i + 1) & 0xFF;
        } while (i < P16(0xA));
    }
    if (BREC(226, state) == 2) {
        BREC(226, state) = 0;
        BREC_BYTE0 = 0;
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        return;
    }
    if (BREC(227, state) == 2) {
        BREC(227, state) = 0;
        BREC_BYTE0 = 0;
        SI(list_mark_mode) = SI(list_mark_mode) == 0;
        list_b_marks_clear();
    } else if (BREC(228, state) == 2) {
        u32 v;
        BREC(228, state) = 0;
        BREC_BYTE0 = 0;
        v = SYS8(0x84D6 + PS16(0xE));
        if (SI(list_mark_mode) == 0) {
            if (v) {
                ui_window_invalidate_from(0x18, v);
                return;
            }
            SI(list_mark_mode) = 1;
            list_b_page_mark();
        } else if (v == 0) {
            list_b_page_mark();
        } else {
            SI(list_mark_mode) = 0;
            list_b_marks_clear();
        }
    } else if (BREC(229, state) == 2) {
        BREC(229, state) = 0;
        BREC_BYTE0 = 0;
        if (SI(list_mark_mode)) {
            list_b_marks_count();
            ui_window_open(0x19);
        }
        return;
    } else {
        if (BREC(230, state) == 2) {
            BREC(230, state) = 0;
            BREC_BYTE0 = 0;
            nv = PS16(0x14) - P16(0xA);
        } else if (BREC(231, state) == 2) {
            BREC(231, state) = 0;
            BREC_BYTE0 = 0;
            nv = P16(0xA) + PS16(0x14);
        } else {
            return;
        }
        PS16(0x14) = (short)nv;
        PS16(0xE) = (short)sdv((short)nv, P16(0xA));
        list_b_cursor_clamp();
        list_b_page_thumbs();
    }
    inv_left(0x18);
}
