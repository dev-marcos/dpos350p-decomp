/*
 * USBC_ForceVbusValidDisable -- stock 0x4006C1DC (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ISCR: Force VBUS valid off.
 */
#include "hw/usb.h"

u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val);

void USBC_ForceVbusValidDisable(void)
{
    u32 v = REG32((USBC0_BASE + 0x400));
    REG32((USBC0_BASE + 0x400)) = USBC_WakeUp_ClearChangeDetect(v & ~0x3000u);
}
