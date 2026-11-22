/*
 * fpga_param_updater -- stock 0x40049EC4 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sends job idx with the value (the 32-bit one for size 4, else the narrow one) unless that value was already sent. (name: Hack SMEC.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"

void fpga_param_updater(u32 idx, u32 size, u32 v32, u32 vsmall)
{
    u32 v = size < 4 ? vsmall : v32;
    smec_state_t *s = g_smec;
    if (s->cache[idx] != v || s->valid[idx] == 0) {
        s->valid[idx] = 1;
        s->cache[idx] = v;
        smec_run_job(idx, v);
        s->pending = 1;
    }
}
