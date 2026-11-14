/*
 * pe0_clr -- stock 0x4003FA04 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PE0 low.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void pe0_clr(void)
{
    pio_clr_bit((void *)0x01C20890u, 1);
}
