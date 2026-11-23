/*
 * zoom_index_fit -- stock 0x400423B8 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Zoom index byte SYS+0x598 (limited to below SYS+0x588): selects the entry (zoom_entry_select) and, while u64 SYS+0x590 / u64 SYS+0x5A0 / word SYS+0x5D4 is at least u16 SYS+0x2AC, steps the index up and invalidates window 3 (r1 = high word of the quotient). Finally the index is clamped to SYS+0x2A8 .. SYS+0x2A7.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/windows.h"
#include "sys_info.h"

#define Z64(o) (*(volatile unsigned long long *)(0x40544E68u + 0x590u + (o)))
#define Z8(o) (*(volatile unsigned char *)(0x40544E68u + 0x590u + (o)))

void zoom_index_fit(void)
{
    unsigned long long q;
    u32 k, lo, hi;
    if (Z8(8) >= SI(timebase_index))
        Z8(8) = SI(timebase_index) - 1;
    for (;;) {
        zoom_entry_select();
        q = Z64(0) / Z64(0x10);
        q = q / *(volatile u32 *)(0x40544E68u + 0x590u + 0x44);
        k = Z8(8);
        if ((u32)(q >> 32) == 0 && (u32)q < SI(f_2ac))
            break;
        Z8(8) = k + 1;
        ui_window_invalidate_from(3, (u32)(q >> 32));
    }
    lo = SI(timebase_index_min);
    if (k < lo)
        k = lo;
    hi = SI(timebase_index_max);
    if (hi > k)
        hi = k;
    Z8(8) = hi;
}
