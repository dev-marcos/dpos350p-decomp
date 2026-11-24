/*
 * memset_w -- stock 0x400356E4 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * memset(dst, value, n): b __aeabi_memset (0x4005C604) with the length and value registers swapped.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "util.h"
#include "shared/framebuffer.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void *__aeabi_memset_entry_raw(void *d, unsigned int n, int v) __asm__("__aeabi_memset_entry");

void *memset_w(void *d, int v, unsigned int n) { return __aeabi_memset_entry_raw(d, n, v); }
