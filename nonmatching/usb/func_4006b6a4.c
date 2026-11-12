/*
 * usb_device_init -- stock 0x4006B6A4 (128 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Start the device as class type (1 HID, 2 CDC, 4 mass storage): BSP init, disable, IRQ 103, enable. -1 if it already runs as that type.
 */
#include "hw/usb.h"

int usb_dev_bsp_init(void);
void sunxi_udc_disable(void);
void sunxi_udc_enable(void);
void usb_irq_handle(void);

int usb_device_init(unsigned char type)
{
    dbg_printf(STR(0x4008FA1C));
    if (current_usb_type == type) {
        dbg_printf(STR(0x4008EFA0));
        return -1;
    }
    current_usb_type = type;
    if (usb_dev_bsp_init() != 0) {
        debug_printf_stub(STR(0x4008DC00));
        return -1;
    }
    sunxi_udc_disable();
    irq_register(0x67, usb_irq_handle, 0, 1);
    irq_enable();
    sunxi_udc_enable();
    dbg_printf(STR(0x4008EF8C));
    return 0;
}
