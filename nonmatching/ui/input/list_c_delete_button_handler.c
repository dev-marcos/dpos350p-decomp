/*
 * list_c_delete_button_handler -- stock 0x400555E8 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x9EE released: clears it and the touch byte, deletes the marked records of list C and closes window 0x1B.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/input.h"
#include "ui/windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))

void list_c_delete_button_handler(void)
{
    touch_button_update_09EE();
    if (TB(0x9EE) != 2)
        return;
    TB(0x9EE) = 0;
    TB(0) = 0;
    list_c_delete_marked();
    ui_window_close_raw(0x1B);
}
