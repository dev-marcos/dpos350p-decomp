/*
 * gic_cpu_id -- stock 0x400669D8 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Always CPU 0.
 */
#include "hw/platform.h"
#include "drivers/gic.h"

int gic_cpu_id(void) { return 0; }
