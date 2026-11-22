/*
 * gt911_bus_init -- stock 0x400498A4 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: reset sequence, INT to input, wait 20 ms (Hack: bus initialization wrapper).
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"
#include "drivers/timer.h"

void gt911_bus_init(void)
{
    gt911_reset_seq();
    gt911_int_input();
    delay_ms(20);
}
