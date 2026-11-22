/*
 * usb_phy_open_clock_Device -- stock 0x4006C384 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * USB PHY clock gate+reset (CCU 0x01C200CC bits 8, 0), bus gate (0x01C20060 bit 24) and bus reset (0x01C202C0 bit 24). FNIRSI version of usb_phy_open_clock.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/phy.h"

u32 usb_phy_open_clock_Device(void)
{
    debug_printf_stub(STR(0x4008E27C));
    REG32(0x01C200CCu) |= 0x100;
    REG32(0x01C200CCu) |= 1;
    REG32(0x01C20060u) |= 0x1000000;
    REG32(0x01C202C0u) |= 0x1000000;
    return 0;
}
