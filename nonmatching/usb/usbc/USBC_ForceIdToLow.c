/*
 * USBC_ForceIdToLow -- stock 0x4006C154 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ISCR: Force ID = 10 (host).
 */
#include "hw/usb.h"

u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val);

void USBC_ForceIdToLow(void)
{
    u32 v = REG32((USBC0_BASE + 0x400));
    REG32((USBC0_BASE + 0x400)) = USBC_WakeUp_ClearChangeDetect((v & ~0xC000u) | 0x8000);
}
