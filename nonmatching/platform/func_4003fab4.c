/*
 * veneer_40063714 -- stock 0x4003FAB4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to 0x40063714.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void veneer_40063714(void)
{
    func_40063714();
}
