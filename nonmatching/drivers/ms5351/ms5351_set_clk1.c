/*
 * ms5351_set_clk1 -- stock 0x4005F38C (420 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: set CLK1 to freq Hz (PLLB regs 0x22, MS1 regs 0x32, PLL reset 0xB1=0x80, CLK1 control 0x11=0x6F); stores the VCO frequency in the global at 0x42E68C68.
 */
#include "hw/drivers.h"

#define PE ((pio_port_t *)PIO_PE)

#include "hw/ms5351_calc.h"
#include "drivers/ms5351.h"

void ms5351_set_clk1(u32 freq)
{
    ms_plan p;
    ms5351_plan(freq, &p);
    REG32(0x42E68C68u) = p.vco;
    ms5351_write_pll(0x22, p.a, p.b, 0xFFFFF);
    if (!p.high) ms5351_write_ms_int(0x32, p.ms_div, p.rdiv_bits);
    else ms5351_write_ms_int(0x32, 4, 0x0C);
    ms5351_write_reg(0xB1, 0x80);
    ms5351_write_reg(0x11, 0x6F);
}
