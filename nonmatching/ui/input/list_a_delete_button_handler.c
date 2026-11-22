/*
 * list_a_delete_button_handler -- stock 0x40054EB8 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x7FA released: clears it and the touch byte, deletes the marked entries of list A, closes window 0x15, redraws the thumbnails and reloads the current entry.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_button_update_07FA(void);
void list_a_delete_marked(void);
void list_a_page_thumbs(void);
void list_a_current_load(void);

void list_a_delete_button_handler(void)
{
    touch_button_update_07FA();
    if (TB(0x7FA) != 2)
        return;
    TB(0x7FA) = 0;
    TB(0) = 0;
    list_a_delete_marked();
    ui_window_close(0x15);
    list_a_page_thumbs();
    list_a_current_load();
}
