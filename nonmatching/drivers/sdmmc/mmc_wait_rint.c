/*
 * mmc_wait_rint -- stock 0x400676BC (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Poll RINTSTS (+0x38) until a mask bit is set (5 us steps); an error bit (0xBFC2) or timeout gives a message and -0x98.
 */
#include "hw/platform.h"

u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
void mmc_delay_us(u32 us);
u32 mmc_base(u32 n);
int mmc_update_clk(u32 n);
#define mmc_capacity (*(u32 *)0x42E68CC0u)

int mmc_wait_rint(u32 n, u32 timeout, u32 mask)
{
    u32 base = mmc_base(n), v;
    for (;;) {
        v = readl_a(base + 0x38);
        if (timeout-- == 0 || (v & 0xBFC2)) {
            dbg_printf(STR(0x4008D8AC), n, timeout, v & 0xBFC2);
            return -0x98;
        }
        mmc_delay_us(5);
        if (v & mask)
            return 0;
    }
}
