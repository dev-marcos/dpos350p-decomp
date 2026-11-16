/*
 * touch_btn_01ec_release -- stock 0x40055B98 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Updates button +0x1EC (0x4002FDB4); on its release clears it and the table byte +0, then runs the factory reset 0x400447C8 (it does not return).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define BTN8(o)  (*(volatile unsigned char *)(0x42E61FF8u + (o)))   /* button table */
void ui_window_open(u32 id);

void touch_button_update_01EC(void);
void func_400447c8(void);

void touch_btn_01ec_release(void)
{
    touch_button_update_01EC();
    if (BTN8(0x1EC) != 2)
        return;
    BTN8(0x1EC) = 0;
    BTN8(0) = 0;
    func_400447c8();
}
