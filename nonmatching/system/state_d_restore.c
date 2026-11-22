/*
 * state_d_restore -- stock 0x4003F0A8 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the block 0x42E68668 back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void state_d_restore(void)
{
    memcpy((void *)0x42E68668u, (void *)0x42E68800u, 0x198);
}
