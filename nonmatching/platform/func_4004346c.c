/*
 * spi0_clock_on -- stock 0x4004346C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU gate for SPI0 (ccu_gate_on(0x0C145814)).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ccu_gate_on(u32 id);

void spi0_clock_on(void)
{
    ccu_gate_on(0x0C145814u);
}
