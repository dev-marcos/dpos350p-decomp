/*
 * func_40032e0c_both -- stock 0x40034058 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40032D3C, then 0x40032E0C(0) and 0x40032E0C(1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40032d3c(void);
void func_40032e0c(u32 a);

void func_40032e0c_both(void)
{
    func_40032d3c();
    func_40032e0c(0);
    func_40032e0c(1);
}
