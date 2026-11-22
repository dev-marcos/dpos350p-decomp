/*
 * list_a_delete_button_handler -- stock 0x40054EB8 (80 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x7FA released: clears it and the touch byte, deletes the marked entries of list A, closes window 0x15, redraws the thumbnails and reloads the current entry.
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

void list_a_delete_button_handler(void)
{
    touch_button_update_07FA();
    if (TB(0x7FA) != 2)
        return;
    TB(0x7FA) = 0;
    TB(0) = 0;
    list_a_delete_marked();
    ui_window_close_raw(0x15);
    list_a_page_thumbs();
    list_a_current_load();
}
