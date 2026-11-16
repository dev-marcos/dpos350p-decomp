/*
 * outputs_apply -- stock 0x4003E708 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set: 0x4003A654, frontend_apply_all, 0x40063328.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void acquisition_apply(void);
void frontend_apply_all(void);
void func_40063328(void);

void outputs_apply(void)
{
    if ((*(volatile unsigned char *)0x42E68BC7u))
        return;
    acquisition_apply();
    frontend_apply_all();
    func_40063328();
}
