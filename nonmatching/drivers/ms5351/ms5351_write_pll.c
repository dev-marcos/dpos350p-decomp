/*
 * ms5351_write_pll -- stock 0x4005F55C (232 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: program a PLL/multisynth block (8 registers at base) from a + b/c, Si5351 encoding: P1 = 128a + floor(128b/c) - 512, P2 = 128b - c*floor(128b/c), P3 = c. floor(128b/c) is computed in single precision (VCVT.U32.F32 #7).
 */
#include "hw/drivers.h"
#include "drivers/ms5351.h"
#include "shared/drivers.h"

void ms5351_write_pll(u32 base, u32 a, u32 b, u32 c)
{
    float q = (float)b / (float)c;
    u32 f = (u32)(q * 128.0f);
    u32 p1 = f + (a << 7) - 512;
    u32 p2 = (b << 7) - f * c;
    ms5351_write_reg(base, (c << 16) >> 24);
    ms5351_write_reg((base + 1) & 0xFF, c & 0xFF);
    ms5351_write_reg((base + 2) & 0xFF, (p1 >> 16) & 3);
    ms5351_write_reg((base + 3) & 0xFF, (p1 << 16) >> 24);
    ms5351_write_reg((base + 4) & 0xFF, p1 & 0xFF);
    ms5351_write_reg((base + 5) & 0xFF, ((p2 >> 16) & 0xF) | ((c >> 12) & 0xF0));
    ms5351_write_reg((base + 6) & 0xFF, (p2 << 16) >> 24);
    ms5351_write_reg((base + 7) & 0xFF, p2 & 0xFF);
}
