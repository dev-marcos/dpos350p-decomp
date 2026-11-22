/*
 * gen_update -- stock 0x4004D18C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator update: 0x4004EEEC, limits, FPGA jobs, table build (0x4004D880), 0x4004ED18, table repeat.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void gen_update(void)
{
    register u32 r0 asm("r0"), r1 asm("r1"), r2 asm("r2"), r3 asm("r3");
    fpga_cmd63_write();
    asm volatile("" : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3));   /* gen_limits gets whatever r0..r3 hold, as in the stock code */
    gen_limits(r0, r1, r2, r3);
    gen_send_params();
    gen_wave_build();
    gen_fpga_settings_write();
    gen_table_repeat_default();
}
