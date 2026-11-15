/*
 * full_acquisition_veneer -- stock 0x40041918 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Branch to full_acquisition_entry.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void full_acquisition_entry(void);

void full_acquisition_veneer(void)
{
    full_acquisition_entry();
}
