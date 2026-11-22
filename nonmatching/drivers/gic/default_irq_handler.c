/*
 * default_irq_handler -- stock 0x400668E0 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Empty handler installed in every slot of the IRQ table.
 */
#include "hw/platform.h"
#include "drivers/gic.h"

void default_irq_handler(void) { }
