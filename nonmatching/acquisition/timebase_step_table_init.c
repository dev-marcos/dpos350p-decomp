/*
 * timebase_step_table_init -- stock 0x4003A680 (572 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the 36 s64 entries at SYS+0xBD0 with the 1-2-5 series 1, 2, 5, 10 .. 5e13 (the time-base steps read by timebase_period_load). Entry 11 (SYS+0xC28) is 5000000 instead of 5000 in this build.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

#define SYS 0x40544E68u

void timebase_step_table_init(void)
{
    SI(f_bd0) = 1LL;
    SI(f_bd8) = 2LL;
    SI(f_be0) = 5LL;
    SI(f_be8) = 10LL;
    SI(f_bf0) = 20LL;
    SI(f_bf8) = 50LL;
    SI(f_c00) = 100LL;
    SI(f_c08) = 200LL;
    SI(f_c10) = 500LL;
    SI(f_c18) = 1000LL;
    SI(f_c20) = 2000LL;
    SI(f_c28) = 5000000LL;      /* sic: the series would give 5000 */
    SI(f_c30) = 5000LL;
    SI(f_c38) = 10000LL;
    SI(f_c40) = 20000LL;
    SI(f_c48) = 50000LL;
    SI(f_c50) = 100000LL;
    SI(f_c58) = 200000LL;
    SI(f_c60) = 500000LL;
    SI(f_c68) = 1000000LL;
    SI(f_c70) = 2000000LL;
    SI(f_c78) = 5000000LL;
    SI(f_c80) = 10000000LL;
    SI(f_c88) = 20000000LL;
    SI(f_c90) = 50000000LL;
    SI(f_c98) = 100000000LL;
    SI(f_ca0) = 200000000LL;
    SI(f_ca8) = 500000000LL;
    SI(f_cb0) = 1000000000LL;
    SI(f_cb8) = 2000000000LL;
    SI(f_cc0) = 5000000000LL;
    SI(f_cc8) = 10000000000LL;
    SI(f_cd0) = 20000000000LL;
    SI(f_cd8) = 50000000000LL;
    SI(f_ce0) = 100000000000LL;
    SI(f_ce8) = 200000000000LL;
    SI(f_cf0) = 500000000000LL;
    SI(f_cf8) = 1000000000000LL;
    SI(f_d00) = 2000000000000LL;
    SI(f_d08) = 5000000000000LL;
    SI(f_d10) = 10000000000000LL;
    SI(f_d18) = 20000000000000LL;
    SI(f_d20) = 50000000000000LL;
}
