/*
 * list_cursor_clamp_40035248 -- stock 0x40035248 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * s16 SYS+0xFA4 is clamped to 0..s16 SYS+0xFA0 - 1 (always stored); s16 SYS+0xF9E is set to 0 when negative, or to s16 SYS+0xFA2 - 1 when above it.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"

#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))

void list_cursor_clamp_40035248(void)
{
    int v = SS16(0xFA4), m;
    if (v < 0) {
        v = 0;
    } else {
        m = SS16(0xFA0) - 1;
        if (m < v)
            v = m;
    }
    SS16(0xFA4) = v;
    v = SS16(0xF9E);
    if (v < 0) {
        SS16(0xF9E) = 0;
    } else {
        m = SS16(0xFA2) - 1;
        if (m < v)
            SS16(0xF9E) = m;
    }
}
