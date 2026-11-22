/*
 * list_c_window_buttons -- stock 0x400553A0 (584 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * List C window, the same layout as list A / B: entry buttons 0x91C + 10i select an entry; outside mark mode the entry's record number (halfword SYS+0xFA6 + 2 * entry) goes to halfword 0x4091F396 (SYS+0x3DA52E), the index is saved, that record selected and the generator samples loaded and applied. 0x9B2 back; 0x9BC mark mode; 0x9C6 mark page; 0x9D0 opens the delete window 0x1B when marking; 0x9DA / 0x9E4 page -/+. Most paths end by invalidating from window 0x1A.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_c_index_save(void);
void list_c_record_select(u32 v);
void gen_load_samples(void);
void gen_update(void);
void list_c_marks_clear(void);
void list_c_page_mark(void);
void list_count_nonzero_40037880(void);
void list_cursor_clamp_4003771c(void);
#define LC_REC (*(volatile unsigned short *)(0x40544E68u + 0x3DA52E))
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

void touch_buttons_91c_update(void);

void list_c_window_buttons(void)
{
    u32 i;
    int nv;
    touch_buttons_91c_update();
    if (P16(0xA)) {
        i = 0;
        do {
            if (TB(0x91C + 10 * i) == 2) {
                int ps, tot, c;
                TB(0x91C + 10 * i) = 0;
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
                        LC_REC = *(volatile unsigned short *)(0x40544E68u + 0xFA6 + 2 * c);
                        list_c_index_save();
                        list_c_record_select(LC_REC);
                        gen_load_samples();
                        gen_update();
                    }
                }
            }
            i = (i + 1) & 0xFF;
        } while (i < P16(0xA));
    }
    if (TB(0x9B2) == 2) {
        TB(0x9B2) = 0;
        TB(0) = 0;
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
        return;
    }
    if (TB(0x9BC) == 2) {
        TB(0x9BC) = 0;
        TB(0) = 0;
        SYS8(0xF90) = SYS8(0xF90) == 0;
        list_c_marks_clear();
    } else if (TB(0x9C6) == 2) {
        u32 v;
        TB(0x9C6) = 0;
        TB(0) = 0;
        v = SYS8(0x84D6 + PS16(0xE));
        if (SYS8(0xF90) == 0) {
            if (v) {
                ui_window_invalidate_from(0x1A, v);
                return;
            }
            SYS8(0xF90) = 1;
            list_c_page_mark();
        } else if (v == 0) {
            list_c_page_mark();
        } else {
            SYS8(0xF90) = 0;
            list_c_marks_clear();
        }
    } else if (TB(0x9D0) == 2) {
        TB(0x9D0) = 0;
        TB(0) = 0;
        if (SYS8(0xF90)) {
            list_count_nonzero_40037880();
            ui_window_open(0x1B);
        }
        return;
    } else {
        if (TB(0x9DA) == 2) {
            TB(0x9DA) = 0;
            TB(0) = 0;
            nv = PS16(0x14) - P16(0xA);
        } else if (TB(0x9E4) == 2) {
            TB(0x9E4) = 0;
            TB(0) = 0;
            nv = P16(0xA) + PS16(0x14);
        } else {
            return;
        }
        PS16(0x14) = (short)nv;
        PS16(0xE) = (short)sdv((short)nv, P16(0xA));
        list_cursor_clamp_4003771c();
    }
    inv_left(0x1A);
}
