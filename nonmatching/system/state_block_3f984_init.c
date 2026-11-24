/*
 * state_block_3f984_init -- stock 0x4003ED28 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Defaults of the block at 0x42E3F984 (+1 = 0, +4 = pointer to +8, +8 = 0, +0xC = 0, +0x2EFA = 1, +0x2EFC/+0x2F00 = 1000, +0x2F04 = 0, +0x2F08 = 500, +0x2F60 = 0) and the byte 0x42E68BC6 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "state/gen_block.h"

void state_block_3f984_init(void)
{
    GENS(f_001) = 0;
    GENS(f_004) = 0x42E3F984u + 8;
    GENS(waveform) = 1;
    GENS(frequency) = 1000;
    GENS(amplitude) = 1000;
    GENS(f_008) = 0;
    GENS(offset) = 0;
    GENS(f_00c) = 0;
    GENS(f_2f60) = 0;
    GENS(f_2f08) = 500;
    g_touch->enabled = 1;
}
