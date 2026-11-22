/*
 * timebase_menu_buttons -- stock 0x400533A0 (1060 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch handler of the time-base menu: touch_group_0426 updates the records, then the first record in state 2 among 0x42E61FF8 + 0x426 (setting 0x2A) and + 0x430 + 0xA x k (setting 0x29 - k, k = 0 .. 29) is reset to 0 together with the flag byte 0x42E61FF8, and the setting is applied: timebase_index_set(v), or zoom_index_set(v) when the split-screen byte SYS+0x2AA is set; then acquisition_apply(), ui_window_invalidate_from(3) and ui_window_close(0xF).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records; byte 0 is the pending-touch flag */
#define B8(a) (*(volatile unsigned char *)(a))

void touch_group_0426(void);
void timebase_index_set(u32 v);
void zoom_index_set(u32 v);
void acquisition_apply(void);
void ui_window_invalidate_from(u32 n);
void ui_window_close(u32 n);

void timebase_menu_buttons(void)
{
    u32 v, rec;
    int k;

    touch_group_0426();
    for (k = -1; k < 30; k++) {
        rec = k < 0 ? 0x426 : 0x430 + 0xA * k;
        if (B8(BTN + rec) == 2) {
            B8(BTN + rec) = 0;
            v = 0x29 - k;
            B8(BTN) = 0;
            if (!B8(SYS + 0x2AA))
                timebase_index_set(v);
            else
                zoom_index_set(v);
            acquisition_apply();
            ui_window_invalidate_from(3);
            ui_window_close(0xF);
            return;
        }
    }
}
