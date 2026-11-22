/*
 * cursor_3da534_clamp -- stock 0x400367EC (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Cursor pair a = word SYS+0x3DA534, b = word SYS+0x3DA538, width w = SYS+0x3DA540: when b < a + w, a = b - w (flag 0) or b = a + w (flag non-zero). Then both are clamped to 12 .. u16 SYS+8 - 12.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "util.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */

void cursor_3da534_clamp(int flag)
{
    int a = SYS32(0x3DA534), w = SYS32(0x3DA540), b = SYS32(0x3DA538);
    if (b < (int)((u32)w + (u32)a)) {
        if (flag == 0)
            a = (int)((u32)b - (u32)w);
        else
            b = (int)((u32)w + (u32)a);
    }
    if (a < 12)
        a = 12;
    else if (SYS16(8) - 12 < a)
        a = SYS16(8) - 12;
    if (b < 12)
        b = 12;
    else if (SYS16(8) - 12 < b)
        b = SYS16(8) - 12;
    SYS32(0x3DA534) = a;
    SYS32(0x3DA538) = b;
}
