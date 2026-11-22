/*
 * veneer_40063798 -- stock 0x4003FAE4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to 0x40063798.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"

void veneer_40063798(void)
{
    fpga_position_290_update();
}
