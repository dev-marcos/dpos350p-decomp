/*
 * state_c_backup -- stock 0x4003F044 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies the block 0x42E180C8 (0xAD20 bytes) into 0x42E22DE8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "util.h"

void state_c_backup(void)
{
    memcpy((void *)0x42E22DE8u, (void *)0x42E180C8u, 0xAD20);
}
