/*
 * measure_select_buttons -- stock 0x400529F0 (1304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch handler of the measurement selection menu: measure_select_buttons_update updates the 24 records 0x42E61FF8 + 0x250 + 0xA x i, then the first record in state 2 (released) is reset to 0 together with the flag byte 0x42E61FF8, and measurement k = i % 12 + 1 of channel c = i / 12 is toggled: removed with 0x400300E8(c, k) when marker_value_present(c, k) says it is shown, else added with marker_value_add(c, k). Nothing happens when no record is in state 2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "stock_asm.h"
#include "ui/input.h"
#include "util.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records; byte 0 is the pending-touch flag */

void measure_select_buttons(void)
{
    u32 c;
    int i, k;

    measure_select_buttons_update();
    for (i = 0; i < 24; i++) {
        if (B8(BTN + 0x250 + 0xA * i) == 2) {
            B8(BTN + 0x250 + 0xA * i) = 0;
            B8(BTN) = 0;
            c = i / 12;
            k = i % 12 + 1;
            if (marker_value_present(c, k))
                func_400300e8(c, k);
            else
                marker_value_add(c, k);
            return;
        }
    }
}
