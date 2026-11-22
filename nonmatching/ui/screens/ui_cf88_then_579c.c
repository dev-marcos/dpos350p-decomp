/*
 * ui_cf88_then_579c -- stock 0x4003D454 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Calls 0x4003CF88 (role OPEN) then ui_text_init (0x4004579C).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ui_layout_init(void);
void ui_text_init(void);

void ui_cf88_then_579c(void)
{
    ui_layout_init();
    ui_text_init();
}
