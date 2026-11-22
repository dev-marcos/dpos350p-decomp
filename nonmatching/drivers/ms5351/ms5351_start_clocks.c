/*
 * ms5351_start_clocks -- stock 0x400490B8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M outputs: CLK0 95 MHz, CLK1 10 MHz.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/ms5351.h"

void ms5351_start_clocks(void)
{
    ms5351_pins_init();
    ms5351_set_clk0(95000000u);
    ms5351_set_clk1(10000000u);
}
