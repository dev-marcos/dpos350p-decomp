/*
 * timebase_apply -- stock 0x4003F894 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Time-base set-up (0x4003F2F4, 0x4003F160) and the three 0x40063xxx helpers.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "stock_asm.h"

void timebase_apply(void)
{
    func_4003f2f4();
    fpga_params_6e4_sync();
    adc_lanes_config_for_rate();
    smec_byte_params_sync();
    func_40063c34_all8();
}
