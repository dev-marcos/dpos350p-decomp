/*
 * Gen_Init -- stock 0x4004D164 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads the generator files (0x40036B00, 0x40036BDC), applies the value at 0x4091F396, loads the samples, then continues into gen_update (the stock code falls through). (name: Hack FPGA.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "storage/files.h"

void Gen_Init(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_record_select(*(volatile unsigned short *)0x4091F396u);
    gen_load_samples();
    gen_update();
}
