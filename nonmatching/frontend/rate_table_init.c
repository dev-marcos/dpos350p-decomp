/*
 * rate_table_init -- stock 0x4004A550 (184 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * u64 table at SYS_INFO+0xEF0: 20 M, 10 M, 5 M, 2 M, 1 M, 500 k, 200 k, 100 k, 50 k, 20 k, 10 k, 5 k, 2 k (indexed by the V/div index; values in uV per division before the probe factor).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void rate_table_init(void)
{
    static const u32 v[13] = { 20000000u, 10000000u, 5000000u, 2000000u, 1000000u, 500000u,
                               200000u, 100000u, 50000u, 20000u, 10000u, 5000u, 2000u };
    int i;
    for (i = 0; i < 13; i++) {
        SYS32(0xEF0 + 8 * i) = v[i];
        SYS32(0xEF4 + 8 * i) = 0;
    }
}
