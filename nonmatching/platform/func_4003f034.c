/*
 * gen_state_backup -- stock 0x4003F034 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies generator state (0x2F70 bytes) into 0x42E428F4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void gen_state_backup(void)
{
    memcpy((void *)0x42E428F4u, (void *)GEN_G, 0x2F70);
}
