/*
 * usb_phy_close_clock_Device -- stock 0x4006C3D4 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reverse of usb_phy_open_clock_Device.
 */
#include "hw/usb.h"
#include "drivers/uart.h"
#include "usb/phy.h"

u32 usb_phy_close_clock_Device(void)
{
    debug_printf_stub(STR(0x4008E298));
    REG32(0x01C200CCu) &= ~0x100u;
    REG32(0x01C200CCu) &= ~1u;
    REG32(0x01C20060u) &= ~0x1000000u;
    REG32(0x01C202C0u) &= ~0x1000000u;
    return 0;
}
