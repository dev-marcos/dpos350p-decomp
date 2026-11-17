/*
 * func_40023ea4_or_40024c60 -- stock 0x40024E14 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x652 == 0: the gate of 0x40023EA4, else the gate of 0x40024C60.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40023ea4_unless_t17(void);
void func_40024c60_unless_t17(void);

void func_40023ea4_or_40024c60(void)
{
    if (SYS8(0x652) == 0)
        func_40023ea4_unless_t17();
    else
        func_40024c60_unless_t17();
}
