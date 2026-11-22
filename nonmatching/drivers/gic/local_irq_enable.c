/*
 * local_irq_enable -- stock 0x40066B88 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b local_irq_enable_body (stock name irq_enable).
 */
#include "hw/platform.h"

void local_irq_enable_body(void);

void local_irq_enable(void) { local_irq_enable_body(); }
