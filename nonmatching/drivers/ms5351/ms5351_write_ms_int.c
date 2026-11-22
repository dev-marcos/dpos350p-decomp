/*
 * ms5351_write_ms_int -- stock 0x4005F644 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: program an integer output multisynth (P1 = 128*div - 512, P2 = 0, P3 = 1) with the R-divider bits in register base+2.
 */
#include "hw/drivers.h"
#include "drivers/ms5351.h"

#define PE ((pio_port_t *)PIO_PE)

void ms5351_write_ms_int(u32 base, u32 div, u32 rdiv_bits)
{
    u32 p1 = (div << 7) - 512;
    ms5351_write_reg(base, 0);
    ms5351_write_reg((base + 1) & 0xFF, 1);
    ms5351_write_reg((base + 2) & 0xFF, rdiv_bits | ((p1 >> 16) & 3));
    ms5351_write_reg((base + 3) & 0xFF, (p1 << 16) >> 24);
    ms5351_write_reg((base + 4) & 0xFF, p1 & 0xFF);
    ms5351_write_reg((base + 5) & 0xFF, 0);
    ms5351_write_reg((base + 6) & 0xFF, 0);
    ms5351_write_reg((base + 7) & 0xFF, 0);
}
