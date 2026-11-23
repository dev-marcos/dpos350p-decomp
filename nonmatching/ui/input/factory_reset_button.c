/*
 * factory_reset_button -- stock 0x40055B98 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Updates button +0x1EC (0x4002FDB4); on its release clears it and the table byte +0, then runs the factory reset 0x400447C8 (it does not return).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "state/touch_buttons.h"


void factory_reset_button(void)
{
    factory_reset_button_update();
    if (BREC(49, state) != 2)
        return;
    BREC(49, state) = 0;
    BREC_BYTE0 = 0;
    factory_reset_keep_language();
}
