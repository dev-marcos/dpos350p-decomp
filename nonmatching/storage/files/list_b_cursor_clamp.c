/*
 * list_b_cursor_clamp -- stock 0x400362F4 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * s16 SYS+0xFA4 is clamped to 0..s16 SYS+0xFA0 - 1 (always stored); s16 SYS+0xF9E is set to 0 when negative, or to s16 SYS+0xFA2 - 1 when above it.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "sys_info.h"


void list_b_cursor_clamp(void)
{
    int v = SI(list_selected), m;
    if (v < 0) {
        v = 0;
    } else {
        m = SI(list_count) - 1;
        if (m < v)
            v = m;
    }
    SI(list_selected) = v;
    v = SI(list_page_index);
    if (v < 0) {
        SI(list_page_index) = 0;
    } else {
        m = SI(list_page_count) - 1;
        if (m < v)
            SI(list_page_index) = m;
    }
}
