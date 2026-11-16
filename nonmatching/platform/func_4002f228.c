/*
 * ui_window_touch_handle -- stock 0x4002F228 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * With a touch state (0x42E68BB0+0xC) pending: a touch strictly inside window id invalidates the windows from id (the stock r1 is the touch state) and clears the state; otherwise a release outside a window that is not self-contained runs ui_windows_close_untouched and clears the state.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */
#define TCH16(o) (*(volatile short *)(0x42E68BB0u + (o)))
#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WH(p, o) (*(volatile unsigned short *)((p) + (o)))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)

void ui_window_invalidate_from(u32 id, u32 r1);
void ui_windows_close_untouched(void);

void ui_window_touch_handle(u32 id)
{
    u32 st = TCH8(0xC);
    volatile unsigned char *p;
    int x, y, a;
    if (st == 0)
        return;
    p = WIN(id);
    x = TCH16(4);
    a = WH(p, 6);
    if (a < x && x < a + WH(p, 0xA)) {
        a = WH(p, 8);
        y = TCH16(6);
        if (a < y && y < a + WH(p, 0xC)) {
            ui_window_invalidate_from(id, st);
            TCH8(0xC) = 0;
            return;
        }
    }
    if (st == 2 && p[2] == 0) {
        ui_windows_close_untouched();
        TCH8(0xC) = 0;
    }
}
