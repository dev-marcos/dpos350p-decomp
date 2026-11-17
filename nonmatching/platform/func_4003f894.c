/*
 * timebase_apply -- stock 0x4003F894 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base set-up (0x4003F2F4, 0x4003F160) and the three 0x40063xxx helpers.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void timebase_apply(void)
{
    func_4003f2f4();
    func_4003f160();
    func_40063f04();
    func_40063cd0();
    func_40063c34_all8();
}
