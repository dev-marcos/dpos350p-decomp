/*
 * vdiv_labels_update -- stock 0x4004AA04 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Both channel labels.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void vdiv_labels_update(void)
{
    register int r2 asm("r2"), r3 asm("r3");
    asm volatile("" : "=r"(r2), "=r"(r3));        /* the stock code passes whatever r2/r3 hold */
    vdiv_label(0, 0, r2, r3);
    asm volatile("" : "=r"(r2), "=r"(r3));
    vdiv_label(1, 0, r2, r3);
}
