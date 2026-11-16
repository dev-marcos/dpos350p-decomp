/*
 * acquisition_rearm -- stock 0x4003B7B8 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set: 0x4003B834, then 0x4004201C when SYS+0x2AA (second traces) is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003b834(void);
void func_4004201c(void);

void acquisition_rearm(void)
{
    if ((*(volatile unsigned char *)0x42E68BC7u))
        return;
    func_4003b834();
    if (SYS8(0x2AA))
        func_4004201c();
}
