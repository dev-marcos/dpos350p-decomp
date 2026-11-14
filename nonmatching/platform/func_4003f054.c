/*
 * state_d_backup -- stock 0x4003F054 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the block 0x42E68668 (0x198 bytes) into 0x42E68800.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void state_d_backup(void)
{
    memcpy((void *)0x42E68800u, (void *)0x42E68668u, 0x198);
}
