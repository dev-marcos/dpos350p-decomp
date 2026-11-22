/*
 * state_c_restore -- stock 0x4003F098 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the block 0x42E180C8 back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "util.h"

void state_c_restore(void)
{
    memcpy((void *)0x42E180C8u, (void *)0x42E22DE8u, 0xAD20);
}
