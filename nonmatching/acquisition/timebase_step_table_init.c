/*
 * timebase_step_table_init -- stock 0x4003A680 (572 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the 36 s64 entries at SYS+0xBD0 with the 1-2-5 series 1, 2, 5, 10 .. 5e13 (the time-base steps read by timebase_period_load). Entry 11 (SYS+0xC28) is 5000000 instead of 5000 in this build.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))

void timebase_step_table_init(void)
{
    S64(SYS + 0xBD0) = 1LL;
    S64(SYS + 0xBD8) = 2LL;
    S64(SYS + 0xBE0) = 5LL;
    S64(SYS + 0xBE8) = 10LL;
    S64(SYS + 0xBF0) = 20LL;
    S64(SYS + 0xBF8) = 50LL;
    S64(SYS + 0xC00) = 100LL;
    S64(SYS + 0xC08) = 200LL;
    S64(SYS + 0xC10) = 500LL;
    S64(SYS + 0xC18) = 1000LL;
    S64(SYS + 0xC20) = 2000LL;
    S64(SYS + 0xC28) = 5000000LL;      /* sic: the series would give 5000 */
    S64(SYS + 0xC30) = 5000LL;
    S64(SYS + 0xC38) = 10000LL;
    S64(SYS + 0xC40) = 20000LL;
    S64(SYS + 0xC48) = 50000LL;
    S64(SYS + 0xC50) = 100000LL;
    S64(SYS + 0xC58) = 200000LL;
    S64(SYS + 0xC60) = 500000LL;
    S64(SYS + 0xC68) = 1000000LL;
    S64(SYS + 0xC70) = 2000000LL;
    S64(SYS + 0xC78) = 5000000LL;
    S64(SYS + 0xC80) = 10000000LL;
    S64(SYS + 0xC88) = 20000000LL;
    S64(SYS + 0xC90) = 50000000LL;
    S64(SYS + 0xC98) = 100000000LL;
    S64(SYS + 0xCA0) = 200000000LL;
    S64(SYS + 0xCA8) = 500000000LL;
    S64(SYS + 0xCB0) = 1000000000LL;
    S64(SYS + 0xCB8) = 2000000000LL;
    S64(SYS + 0xCC0) = 5000000000LL;
    S64(SYS + 0xCC8) = 10000000000LL;
    S64(SYS + 0xCD0) = 20000000000LL;
    S64(SYS + 0xCD8) = 50000000000LL;
    S64(SYS + 0xCE0) = 100000000000LL;
    S64(SYS + 0xCE8) = 200000000000LL;
    S64(SYS + 0xCF0) = 500000000000LL;
    S64(SYS + 0xCF8) = 1000000000000LL;
    S64(SYS + 0xD00) = 2000000000000LL;
    S64(SYS + 0xD08) = 5000000000000LL;
    S64(SYS + 0xD10) = 10000000000000LL;
    S64(SYS + 0xD18) = 20000000000000LL;
    S64(SYS + 0xD20) = 50000000000000LL;
}
