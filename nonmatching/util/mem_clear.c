/*
 * mem_clear -- stock 0x4005F914 (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * __aeabi_memset_entry(d, n, 0) (enters the stock memset after its fill-byte spreading).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "stock_asm.h"
#include "util.h"

void mem_clear(void *d, unsigned int n)
{
    __aeabi_memset_entry(d, n, 0);
}
