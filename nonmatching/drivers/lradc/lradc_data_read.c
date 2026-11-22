/*
 * lradc_data_read -- stock 0x40068E74 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * LRADC data register 0x01C2280C, low 6 bits (V3s LRADC block at 0x01C22800; read by the battery service).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

u32 readl_c(u32 addr);

u32 lradc_data_read(void)
{
    return readl_c(0x01C2280Cu) & 0x3F;
}
