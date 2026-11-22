/*
 * gen_load_samples -- stock 0x4004D12C (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Sample count from SYS_INFO+0x3DA53C into G+0x2EF8, then that many samples from SYS_INFO+0x3DA548 into G+0x1F56.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "util.h"

void gen_load_samples(void)
{
    G16(0x2EF8) = SYS32(0x3DA53C);
    copy_u16((unsigned short *)(GEN_G + 0x1F56), (const unsigned short *)(0x40544E68u + 0x3DA548), G16(0x2EF8));
}
