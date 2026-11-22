/*
 * ui_window_invalidate_from -- stock 0x4003CD44 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Marks dirty the window id and every window above it on the stack, up to the first empty slot. Stock quirk: when id is not on the stack the start slot is whatever the caller left in r1 (low byte); the C takes it as the second argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
void ui_window_stack_top_update(void);

void ui_window_invalidate_from(u32 id, u32 r1)
{
    u32 i, slot = r1, w;
    for (i = 0; (i & 0xFF) < 20; i++) {
        if (WSTK[2 + i] == id) {
            slot = i;
            break;
        }
    }
    slot &= 0xFF;
    for (; slot < 20; slot++) {
        w = WSTK[2 + slot];
        if (w == 0)
            break;
        WIN(w)[1] = 1;
    }
}
