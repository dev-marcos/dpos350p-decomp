/*
 * filtrate_irq_misc -- stock 0x4006ACDC (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clear VBUS error, session request, connect and SOF; return the other bits.
 */
#include "hw/usb.h"

u32 filtrate_irq_misc(u32 irq_misc)
{
    USBC_INT_ClearMiscPending(0x80);
    USBC_INT_ClearMiscPending(0x40);
    USBC_INT_ClearMiscPending(0x10);
    USBC_INT_ClearMiscPending(0x08);
    return irq_misc & ~0xD8u;
}
