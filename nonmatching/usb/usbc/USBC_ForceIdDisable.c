/*
 * USBC_ForceIdDisable -- stock 0x4006C198 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ISCR: Force ID off.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_ForceIdDisable(void)
{
    u32 v = REG32((USBC0_BASE + 0x400));
    REG32((USBC0_BASE + 0x400)) = USBC_WakeUp_ClearChangeDetect(v & ~0xC000u);
}
