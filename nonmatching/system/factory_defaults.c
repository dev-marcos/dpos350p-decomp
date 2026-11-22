/*
 * factory_defaults -- stock 0x4003EFAC (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears SYS_INFO (0x3DD588 bytes), the block 0x42E3F984 (0x2F70), the window table (0x578), the window stack slots (0x14), 0x42E68BB0 (0x18) and 0x42E68A88 (0x58); then 0x4003E738 and state_blocks_init.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void system_defaults_init(void);
void state_blocks_init(void);

void factory_defaults(void)
{
    memset_w((void *)0x40544E68u, 0, 0x3DD588);
    memset_w((void *)0x42E3F984u, 0, 0x2F70);
    memset_w((void *)0x42E64520u, 0, 0x578);
    memset_w((void *)0x42E68BCAu, 0, 0x14);
    memset_w((void *)0x42E68BB0u, 0, 0x18);
    memset_w((void *)0x42E68A88u, 0, 0x58);
    system_defaults_init();
    state_blocks_init();
}
