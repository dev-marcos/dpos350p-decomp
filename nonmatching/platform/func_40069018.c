/*
 * usb_mode_enter -- stock 0x40069018 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Runs 0x4006A98C, then usb_device_init(4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4006a98c(void);
void usb_device_init(u32 mode);

void usb_mode_enter(void)
{
    func_4006a98c();
    usb_device_init(4);
}
