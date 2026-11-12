/*
 * USBC_ForceId -- stock 0x4006C1B8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 1 = low, 2 = high, anything else = disable.
 */
#include "hw/usb.h"

void USBC_ForceIdToLow(void);
void USBC_ForceIdToHigh(void);
void USBC_ForceIdDisable(void);

void USBC_ForceId(u32 type)
{
    switch (type) {
    case 1: USBC_ForceIdToLow(); break;
    case 2: USBC_ForceIdToHigh(); break;
    default: USBC_ForceIdDisable(); break;
    }
}
