/*
 * gen_state_restore -- stock 0x4003F088 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies generator state back from 0x42E428F4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "util.h"

void gen_state_restore(void)
{
    memcpy((void *)GEN_G, (void *)0x42E428F4u, 0x2F70);
}
