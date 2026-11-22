/*
 * ui_window_close -- stock 0x4003CE50 (188 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Closes window id when it is open: clears +0, +1, +3, +5, removes it from the stack by moving the slots above down one (slot 19 keeps its value), marks every stacked window dirty and updates the stack top. Stock quirk: when id is not on the stack the slot used is whatever the caller left in r2; the C takes it as the third argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/windows.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)

void ui_window_close(u32 id, u32 r1, u32 r2)
{
    volatile unsigned char *w = WIN(id);
    u32 i, slot = r2, k;
    (void)r1;
    if (w[0] == 0)
        return;
    w[0] = 0;
    w[1] = 0;
    w[3] = 0;
    w[5] = 0;
    for (i = 0; i <= 19; i++) {
        if (WSTK[2 + i] == id) {
            slot = i;
            break;
        }
    }
    if (slot < 19) {
        for (k = 19 - slot; k; k--, slot++)
            WSTK[2 + slot] = WSTK[3 + slot];
    }
    for (i = 0; i < 20; i++) {
        k = WSTK[2 + i];
        if (k)
            WIN(k)[1] = 1;
    }
    ui_window_stack_top_update();
}
