/*
 * func_40032e0c_both -- stock 0x40034058 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40032D3C, then 0x40032E0C(0) and 0x40032E0C(1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/input.h"
#include "ui/screens.h"

void func_40032e0c_both(void)
{
    key_repeat_update();
    measure_channel(0);
    measure_channel(1);
}
