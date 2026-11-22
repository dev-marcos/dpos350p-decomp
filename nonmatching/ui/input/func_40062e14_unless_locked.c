/*
 * func_40062e14_unless_locked -- stock 0x40062E00 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, branches to 0x40062E14.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void measure_setup_62e14(void);

void func_40062e14_unless_locked(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    measure_setup_62e14();
}
