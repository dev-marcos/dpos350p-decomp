/*
 * auto_set -- stock 0x4004C7C4 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * AUTO button (buttons_05c_handler): auto_set_from(0x40544E68) (the stock code loads r0 and falls through).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "frontend.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define TCH 0x42E68BB0u
#define S8(a) (*(volatile signed char *)(a))

void auto_set(void)
{
    auto_set_from(SYS);
}
