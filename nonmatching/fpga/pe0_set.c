/*
 * pe0_set -- stock 0x4003F9F8 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PE0 high (FPGA bus strobe line).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/pio.h"
#include "fpga.h"

void pe0_set(void)
{
    pio_set_bit((void *)0x01C20890u, 1);
}
