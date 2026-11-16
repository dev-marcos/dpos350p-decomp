/*
 * ui_windows_reset -- stock 0x4003CF0C (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears +0, +1, +3, +5 of the first 100 window records, clears the 20 stack slots (0x4005F914) and the stack top halfword.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
void ui_window_stack_top_update(void);

void func_4005f914(volatile void *p, u32 n);

void ui_windows_reset(void)
{
    volatile unsigned char *w = WIN(0);
    u32 n;
    for (n = 100; n; n--) {
        w[0] = 0;
        w[1] = 0;
        w[3] = 0;
        w[5] = 0;
        w += 14;
    }
    func_4005f914(WSTK + 2, 20);
    *(volatile unsigned short *)WSTK = 0;
}
