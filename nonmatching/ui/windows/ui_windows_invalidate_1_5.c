/*
 * ui_windows_invalidate_1_5 -- stock 0x4003D6E4 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ui_window_invalidate_from for the ids 1..5. Stock quirk: the hidden second argument (r1) is the caller's r1 for the first call and, for the next ones, the r1 the previous call left; the C models that value (inv_r1_out).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WSTK ((volatile unsigned char *)0x42E68BC8u)
void ui_window_invalidate_from(u32 id, u32 r1);

/* r1 as ui_window_invalidate_from (0x4003CD44) leaves it */
static u32 inv_r1_out(u32 id, u32 r1)
{
    u32 i, slot = r1, n;
    for (i = 0; i < 20; i++) {
        if (WSTK[2 + i] == id) {
            slot = i;
            break;
        }
    }
    slot &= 0xFF;
    if (slot >= 20)
        return slot;
    for (n = 20 - slot; n; n--, slot++)
        if (WSTK[2 + slot] == 0)
            break;
    return n;
}

void ui_windows_invalidate_1_5(u32 r0, u32 r1)
{
    u32 id;
    (void)r0;
    for (id = 1; id <= 5; id++) {
        ui_window_invalidate_from(id, r1);
        r1 = inv_r1_out(id, r1);
    }
}
