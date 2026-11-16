/*
 * ui_windows_close_untouched -- stock 0x4002F194 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * From the window stack top index (0x42E68BC9) down to 0: stops at the first window whose rectangle contains the touch point (edges included); every window before it that is not self-contained (+2) is closed. ui_window_close gets the stock r1 (the +2 byte just read, so 0) and r2 (the last value compared).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TCH8(o)  (*(volatile unsigned char *)(0x42E68BB0u + (o)))   /* touch block */
#define TCH16(o) (*(volatile short *)(0x42E68BB0u + (o)))
#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WH(p, o) (*(volatile unsigned short *)((p) + (o)))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)

void ui_window_close(u32 id, u32 r1, u32 r2);

void ui_windows_close_untouched(void)
{
    int i;
    for (i = WSTK[1]; i >= 0; i--) {
        u32 id = WSTK[2 + i];
        volatile unsigned char *p = WIN(id);
        int x = TCH16(4), y, a, r2;
        a = WH(p, 6);
        r2 = a;
        if (x >= a) {
            a = a + WH(p, 0xA);
            r2 = a;
            if (a >= x) {
                a = WH(p, 8);
                y = TCH16(6);
                r2 = a;
                if (y >= a) {
                    a = a + WH(p, 0xC);
                    r2 = a;
                    if (a >= y)
                        return;
                }
            }
        }
        if (p[2] == 0)
            ui_window_close(id, 0, (u32)r2);
    }
}
