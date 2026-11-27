/*
 * usb_device_init -- stock 0x4006B6A4 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Start the device as class type (1 HID, 2 CDC, 4 mass storage): BSP init, disable, IRQ 103, enable. -1 if it already runs as that type.
 */
#include "hw/usb.h"
#include "drivers/gic.h"
#include "drivers/uart.h"
#include "stock_asm.h"
#include "usb/core.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
int irq_register_raw(int, void (*) (void), int, int) __asm__("irq_register");

int usb_device_init(unsigned char type)
{
    dbg_printf(STR(4008fa1c));
    if (current_usb_type == type) {
        dbg_printf(STR(4008efa0));
        return -1;
    }
    current_usb_type = type;
    if (usb_dev_bsp_init() != 0) {
        debug_printf_stub(STR(4008dc00));
        return -1;
    }
    sunxi_udc_disable();
    irq_register_raw(0x67, usb_irq_handle, 0, 1);
    irq_enable();
    sunxi_udc_enable();
    dbg_printf(STR(4008ef8c));
    return 0;
}
