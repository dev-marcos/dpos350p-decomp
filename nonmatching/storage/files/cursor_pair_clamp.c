/*
 * cursor_pair_clamp -- stock 0x400367EC (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Cursor pair a = word SYS+0x3DA534, b = word SYS+0x3DA538, width w = SYS+0x3DA540: when b < a + w, a = b - w (flag 0) or b = a + w (flag non-zero). Then both are clamped to 12 .. u16 SYS+8 - 12.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/storage.h"

void cursor_pair_clamp(int flag)
{
    int a = SI(cursor_a), w = SI(cursor_gap), b = SI(cursor_b);
    if (b < (int)((u32)w + (u32)a)) {
        if (flag == 0)
            a = (int)((u32)b - (u32)w);
        else
            b = (int)((u32)w + (u32)a);
    }
    if (a < 12)
        a = 12;
    else if (SI(plot_width) - 12 < a)
        a = SI(plot_width) - 12;
    if (b < 12)
        b = 12;
    else if (SI(plot_width) - 12 < b)
        b = SI(plot_width) - 12;
    SI(cursor_a) = a;
    SI(cursor_b) = b;
}
