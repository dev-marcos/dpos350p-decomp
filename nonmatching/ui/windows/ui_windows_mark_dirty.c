/*
 * ui_windows_mark_dirty -- stock 0x4003CDC0 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Marks dirty (+1) every window on the stack, from slot 0 up to the first empty slot.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/windows.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)

void ui_windows_mark_dirty(void)
{
    u32 i, id;
    for (i = 0; i < 20; i++) {
        id = WSTK[2 + i];
        if (id == 0)
            break;
        WIN(id)[1] = 1;
    }
}
