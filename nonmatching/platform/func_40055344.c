/*
 * list_b_delete_button_handler -- stock 0x40055344 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x912 released (state 2): clears it and the touch byte, deletes the marked entries of list B, closes window 0x19 and redraws the thumbnails.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_button_update_0912(void);
void list_b_delete_marked(void);
void list_b_page_thumbs(void);

void list_b_delete_button_handler(void)
{
    touch_button_update_0912();
    if (TB(0x912) != 2)
        return;
    TB(0x912) = 0;
    TB(0) = 0;
    list_b_delete_marked();
    ui_window_close(0x19);
    list_b_page_thumbs();
}
