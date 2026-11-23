/*
 * measure_setup_gated -- stock 0x40062E00 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set, branches to 0x40062E14.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/input.h"

void measure_setup_gated(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    measure_setup_62e14();
}
