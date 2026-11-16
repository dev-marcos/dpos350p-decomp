/*
 * runtime_state_commit_force -- stock 0x4003BEA8 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x38 = 1, then falls into runtime_state_commit_copy (0x4003BEB4).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void runtime_state_commit_copy(void);

void runtime_state_commit_force(void)
{
    SYS8(0x38) = 1;
    runtime_state_commit_copy();
}
