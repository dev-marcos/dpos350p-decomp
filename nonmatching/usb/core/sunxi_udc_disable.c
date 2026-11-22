/*
 * sunxi_udc_disable -- stock 0x4006AD84 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * All USB interrupts off and cleared, then soft disconnect.
 */
#include "hw/usb.h"

void clear_all_irq(void);
void cfg_udc_command(u32 cmd);

void sunxi_udc_disable(void)
{
    debug_printf_stub(STR(0x4008EF00));
    USBC_INT_DisableUsbMiscAll();
    USBC_INT_DisableEpAll(USBC_EP_TYPE_RX);
    USBC_INT_DisableEpAll(USBC_EP_TYPE_TX);
    clear_all_irq();
    cfg_udc_command(2);
}
