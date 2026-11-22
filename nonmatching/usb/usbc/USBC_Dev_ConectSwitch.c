/*
 * USBC_Dev_ConectSwitch -- stock 0x4006C5D8 (44 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PCTL bit 6 (soft connect): set for 1, cleared otherwise.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_Dev_ConectSwitch(u32 is_on)
{
    if (is_on == 1)
        REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) | 0x40);
    else
        REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) & 0xBF);
}
