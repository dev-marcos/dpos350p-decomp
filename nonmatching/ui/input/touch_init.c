/*
 * touch_init -- stock 0x40049A74 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch controller and state; enables the touch service.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/gt911.h"
#include "ui/input.h"

void touch_init(void)
{
    gt911_pins_init();
    gt911_bus_init();
    touch_state_clear();
    g_touch->enabled = 1;
}
