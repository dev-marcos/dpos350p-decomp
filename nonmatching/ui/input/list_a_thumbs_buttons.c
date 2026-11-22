/*
 * list_a_thumbs_buttons -- stock 0x40054C84 (564 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * List A thumbnail window: entry buttons 0x728 + 10i (i below the page size SYS+0xF9A) select entry page * size + i of SYS+0xFA4; when it exists (below SYS+0xFA0 + 1) it toggles its mark (SYS+0x5DC6 + entry) in mark mode SYS+0xF90, else loads it and opens window 0x16. 0x7BE back to the main screens; 0x7C8 toggles mark mode (marks cleared); 0x7D2 marks the page or leaves mark mode; 0x7DC opens the delete window 0x15 when marking; 0x7E6 / 0x7F0 page -/+ (thumbnails redrawn). Most paths end by invalidating from window 0x14.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_a_current_load(void);
void list_a_marks_clear(void);
void list_a_page_mark(void);
void list_count_nonzero_400353a8(void);
void list_cursor_clamp_40035248(void);
void list_a_page_thumbs(void);
#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
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

void touch_buttons_728_update(void);

void list_a_thumbs_buttons(void)
{
    u32 i;
    int nv;
    touch_buttons_728_update();
    if (P16(0xA)) {
        i = 0;
        do {
            if (TB(0x728 + 10 * i) == 2) {
                int ps, tot, c;
                TB(0x728 + 10 * i) = 0;
                TB(0) = 0;
                ps = P16(0xA);
                tot = PS16(0x10);
                PS16(0x14) = (short)(ps * sdv(PS16(0x14), ps) + (int)i);
                c = PS16(0x14);
                if (tot >= c + 1) {
                    if (SYS8(0xF90)) {
                        volatile unsigned char *m = (volatile unsigned char *)(0x40544E68u + 0x5DC6 + c);
                        *m = *m == 0;
                    } else {
                        ui_windows_reset();
                        list_a_current_load();
                        ui_window_open(0x16);
                    }
                }
            }
            i = (i + 1) & 0xFF;
        } while (i < P16(0xA));
    }
    if (TB(0x7BE) == 2) {
        TB(0x7BE) = 0;
        TB(0) = 0;
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        return;
    }
    if (TB(0x7C8) == 2) {
        TB(0x7C8) = 0;
        TB(0) = 0;
        SYS8(0xF90) = SYS8(0xF90) == 0;
        list_a_marks_clear();
    } else if (TB(0x7D2) == 2) {
        u32 v;
        TB(0x7D2) = 0;
        TB(0) = 0;
        v = SYS8(0x84D6 + PS16(0xE));
        if (SYS8(0xF90) == 0) {
            if (v) {
                ui_window_invalidate_from(0x14, v);
                return;
            }
            SYS8(0xF90) = 1;
            list_a_page_mark();
        } else if (v == 0) {
            list_a_page_mark();
        } else {
            SYS8(0xF90) = 0;
            list_a_marks_clear();
        }
    } else if (TB(0x7DC) == 2) {
        TB(0x7DC) = 0;
        TB(0) = 0;
        if (SYS8(0xF90)) {
            list_count_nonzero_400353a8();
            ui_window_open(0x15);
        }
        return;
    } else {
        if (TB(0x7E6) == 2) {
            TB(0x7E6) = 0;
            TB(0) = 0;
            nv = PS16(0x14) - P16(0xA);
        } else if (TB(0x7F0) == 2) {
            TB(0x7F0) = 0;
            TB(0) = 0;
            nv = P16(0xA) + PS16(0x14);
        } else {
            return;
        }
        PS16(0x14) = (short)nv;
        PS16(0xE) = (short)sdv((short)nv, P16(0xA));
        list_cursor_clamp_40035248();
        list_a_page_thumbs();
    }
    inv_left(0x14);
}
