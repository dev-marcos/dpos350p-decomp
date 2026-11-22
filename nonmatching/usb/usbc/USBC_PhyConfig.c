/*
 * USBC_PhyConfig -- stock 0x4006C328 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * PHY tuning: 45 ohm (0x0C), magnitude/rate (0x20), disconnect threshold (0x2A).
 */
#include "hw/usb.h"

void USBC_PhyConfig(void)
{
    usb_phy_write(0x0C, 0x01, 1);
    usb_phy_write(0x20, 0x14, 5);
    usb_phy_write(0x2A, 3, 2);
}
