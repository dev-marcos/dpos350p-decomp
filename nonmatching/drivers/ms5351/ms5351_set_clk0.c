/*
 * ms5351_set_clk0 -- stock 0x4005F1F0 (412 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: set CLK0 to freq Hz (PLLA regs 0x1A, MS0 regs 0x2A, PLL reset 0xB1=0x20, CLK0 control 0x10=0x4F); stores the requested frequency in SYS_INFO+0x5F0.
 */
#include "hw/drivers.h"

#define PE ((pio_port_t *)PIO_PE)

#include "hw/ms5351_calc.h"
#include "drivers/ms5351.h"
#include "sys_info.h"

void ms5351_set_clk0(u32 freq)
{
    ms_plan p;
    SI(sample_clock_hz) = freq;
    ms5351_plan(freq, &p);
    ms5351_write_pll(0x1A, p.a, p.b, 0xFFFFF);
    if (!p.high) ms5351_write_ms_int(0x2A, p.ms_div, p.rdiv_bits);
    else ms5351_write_ms_int(0x2A, 4, 0x0C);
    ms5351_write_reg(0xB1, 0x20);
    ms5351_write_reg(0x10, 0x4F);
}
