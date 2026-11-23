/*
 * list_b_delete_button_handler -- stock 0x40055344 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x912 released (state 2): clears it and the touch byte, deletes the marked entries of list B, closes window 0x19 and redraws the thumbnails.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");


void list_b_delete_button_handler(void)
{
    touch_button_update_0912();
    if (BREC(232, state) != 2)
        return;
    BREC(232, state) = 0;
    BREC_BYTE0 = 0;
    list_b_delete_marked();
    ui_window_close_raw(0x19);
    list_b_page_thumbs();
}
