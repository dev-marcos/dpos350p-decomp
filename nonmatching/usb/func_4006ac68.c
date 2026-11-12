/*
 * usb_dev_bsp_init -- stock 0x4006AC68 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clocks, PHY CSR = 2 around the PHY setup, FIFO base, pull-ups, force ID = device and VBUS valid high, PIO bus.
 */
#include "hw/usb.h"

void USBC_EnableDpDmPullUp(void);
void USBC_EnableIdPullUp(void);
void USBC_ForceId(u32 type);
void USBC_ForceVbusValid(u32 type);

int usb_dev_bsp_init(void)
{
    usb_phy_open_clock_Device();
    REG32(USBC_PHY_CSR) = 2;
    USBC_PhyConfig();
    USBC_ConfigFIFO_Base();
    USBC_EnableDpDmPullUp();
    USBC_EnableIdPullUp();
    USBC_ForceId(2);
    USBC_ForceVbusValid(2);
    USBC_SelectBus(0, 0, 0);
    REG32(USBC_PHY_CSR) = 0;
    return 0;
}
