/*
 * USBC_ForceIdToHigh -- stock 0x4006C178 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ISCR: Force ID = 11 (device).
 */
#include "hw/usb.h"

u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val);

void USBC_ForceIdToHigh(void)
{
    u32 v = REG32((USBC0_BASE + 0x400));
    REG32((USBC0_BASE + 0x400)) = USBC_WakeUp_ClearChangeDetect(v | 0xC000);
}
