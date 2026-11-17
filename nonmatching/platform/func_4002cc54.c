/*
 * func_4002c7ac_or_4002c5f8 -- stock 0x4002CC54 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x652 == 0: 0x4002C7AC, else 0x4002C5F8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4002c7ac(void);
void func_4002c5f8(void);

void func_4002c7ac_or_4002c5f8(void)
{
    if (SYS8(0x652) == 0)
        func_4002c7ac();
    else
        func_4002c5f8();
}
