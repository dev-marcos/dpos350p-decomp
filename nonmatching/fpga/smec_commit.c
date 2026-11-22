/*
 * smec_commit -- stock 0x40049F1C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When jobs were sent since the last commit, sends job idx with value 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"

void smec_commit(u32 idx)
{
    if (g_smec->pending != 0) {
        smec_run_job(idx, 0);
        g_smec->pending = 0;
    }
}
