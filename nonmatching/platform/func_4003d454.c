/*
 * ui_cf88_then_579c -- stock 0x4003D454 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Calls 0x4003CF88 then 0x4004579C (roles not yet identified).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003cf88(void);
void func_4004579c(void);

void ui_cf88_then_579c(void)
{
    func_4003cf88();
    func_4004579c();
}
