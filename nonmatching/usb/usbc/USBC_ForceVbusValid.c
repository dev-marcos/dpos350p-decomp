/*
 * USBC_ForceVbusValid -- stock 0x4006C240 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 1 = low, 2 = high, anything else = disable.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_ForceVbusValid(u32 type)
{
    switch (type) {
    case 1: USBC_ForceVbusValidToLow(); break;
    case 2: USBC_ForceVbusValidToHigh(); break;
    default: USBC_ForceVbusValidDisable(); break;
    }
}
