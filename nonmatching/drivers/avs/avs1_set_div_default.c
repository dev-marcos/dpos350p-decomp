/*
 * avs1_set_div_default -- stock 0x40042D4C (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * avs_set_div(1, 24) (the stock code falls through into avs_set_div).
 */
#include "hw/drivers.h"
#include "drivers/avs.h"
#include "drivers/mmio.h"

void avs1_set_div_default(void)
{
    avs_set_div(1, 0x18);
}
