/*
 * usb_phy_write -- stock 0x4006C2B8 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Bit-bang len bits of data into the PHY through the CSR at +0x410. Every macro does a 32-bit read and an 8-bit write (FNIRSI variant), so the bit address (bits 15:8) never reaches the PHY.
 */
#include "hw/usb.h"

void usb_phy_write(int addr, int data, int len)
{
    int j;
    for (j = 0; j < len; j++) {
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~(0xFFu << 8));
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) | ((u32)(addr + j) << 8));
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~1u);
        if (data & 1)
            REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) | 0x80);
        else
            REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~0x80u);
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) | 1);
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~1u);
        data >>= 1;
    }
}
