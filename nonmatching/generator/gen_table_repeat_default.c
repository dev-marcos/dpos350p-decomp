/*
 * gen_table_repeat_default -- stock 0x4004D094 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * gen_table_repeat on the table descriptor at G+0xFAE. The stock code falls through into it (the analyser split the entry).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"

void gen_table_repeat_default(void)
{
    register u32 r1 asm("r1");
    asm volatile("" : "=r"(r1));
    gen_table_repeat(0xFAE, r1, g_touch);
}
