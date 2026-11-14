/*
 * veneer_400636bc -- stock 0x4003FAB0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to 0x400636BC.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void veneer_400636bc(void)
{
    func_400636bc();
}
