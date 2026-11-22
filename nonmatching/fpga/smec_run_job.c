/*
 * smec_run_job -- stock 0x4004A0B0 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sends job (idx, val) and returns its result. On a missing echo or invalid result the index timing grows by 100 up to 3000; 4 more tries at 3000, then 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"

u32 smec_run_job(u32 idx, u32 val)
{
    volatile u32 *t = g_smec_timing;
    int tries = 0;
    for (;;) {
        u32 echo, ok, v;
        selector_0x69_16_bit_timing_retry(t[idx] & 0xFFFF);
        selector_0x6a_one_byte_job_index(idx);
        selector_0x6b_32_bit_job_value(val);
        smec_start_job();
        smec_wait_ready();
        echo = smec_job_echo();
        ok = smec_job_valid();
        if (echo == idx && ok != 0)
            return smec_job_result();
        v = t[idx] + 100;
        t[idx] = v;
        if (v <= 3000)
            continue;
        tries++;
        t[idx] = 3000;
        if (tries >= 4)
            return 0;
    }
}
