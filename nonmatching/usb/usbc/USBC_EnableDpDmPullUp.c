/*
 * USBC_EnableDpDmPullUp -- stock 0x4006C114 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ISCR: DP/DM pull-up (bit 16) and VBUS valid source 3 (bits 11:10).
 */
#include "hw/usb.h"

u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val);

void USBC_EnableDpDmPullUp(void)
{
    u32 v = REG32((USBC0_BASE + 0x400));
    REG32((USBC0_BASE + 0x400)) = USBC_WakeUp_ClearChangeDetect(v | 0x10C00);
}
