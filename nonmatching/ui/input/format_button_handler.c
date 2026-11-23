/*
 * format_button_handler -- stock 0x40055BD4 (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Button 0x200 released: formats the disk (disk_format), shows the message of text entry 0xAAA0 at the centre of window rectangle 0x1B8 (plus its offsets), waits 500 ms, restores the area and closes window 0x1F.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/timer.h"
#include "storage/diskio.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"
#include "state/ui_windows.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_close_raw(u32 id) __asm__("ui_window_close");

#define TX8(o) (*(volatile signed char *)(0x42C2FE74u + (o)))
#define TX32(o) (*(volatile u32 *)(0x42C2FE74u + (o)))

void format_button_handler(void)
{
    u32 cx, cy;
    touch_button_update_0200();
    cy = WREC(31, y) + (WREC(31, h) >> 1);
    cx = WREC(31, x) + (WREC(31, w) >> 1);
    if (BREC(51, state) != 2)
        return;
    BREC(51, state) = 0;
    BREC_BYTE0 = 0;
    disk_format();
    message_box_draw(TX8(0xAAA0 + 0x194) + (int)(cx & 0xFFFF), TX8(0xAAA0 + 0x195) + (int)(cy & 0xFFFF),
                     TX32(0xAAA0), 0);
    delay_ms(500);
    rect_42e68c38_draw();
    ui_window_close_raw(0x1F);
}
