/*
 * SMEC_Init -- stock 0x4004A15C (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the job state, timing 2000, reset command, sends the acquisition parameters. (name: Hack SMEC.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void SMEC_Init(void)
{
    __aeabi_memclr4(g_smec, 0x8FC);
    smec_timing_reset();
    smec_reset_cmd();
    selector_0x69_16_bit_timing_retry(0x7D0);
    fpga_params_push();
}
