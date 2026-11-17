/*
 * list_c_delete_button_handler -- stock 0x400555E8 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x9EE released: clears it and the touch byte, deletes the marked records of list C and closes window 0x1B.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_button_update_09EE(void);
void list_c_delete_marked(void);

void list_c_delete_button_handler(void)
{
    touch_button_update_09EE();
    if (TB(0x9EE) != 2)
        return;
    TB(0x9EE) = 0;
    TB(0) = 0;
    list_c_delete_marked();
    ui_window_close(0x1B);
}
