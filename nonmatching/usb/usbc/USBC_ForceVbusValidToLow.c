/*
 * USBC_ForceVbusValidToLow -- stock 0x4006C1FC (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ISCR: Force VBUS valid = 10.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_ForceVbusValidToLow(void)
{
    u32 v = REG32((USBC0_BASE + 0x400));
    REG32((USBC0_BASE + 0x400)) = USBC_WakeUp_ClearChangeDetect((v & ~0x3000u) | 0x2000);
}
