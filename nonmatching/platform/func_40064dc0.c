/*
 * check_pair_save -- stock 0x40064DC0 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40063A50; r = 0x40064C28() & 0x40064CF0() & 1; settings_a_save when r; 0x4003EB40; byte 0x42E68AE0+0x45 = r.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40063a50(void);
u32 gen_selftest_64c28(void);
u32 func_40064cf0(void);
void settings_a_save(void);
void func_4003eb40(void);

void check_pair_save(void)
{
    u32 a, r;
    func_40063a50();
    a = gen_selftest_64c28();
    r = func_40064cf0() & a & 1;
    if (r)
        settings_a_save();
    func_4003eb40();
    *(volatile unsigned char *)0x42E68B25u = r;
}
