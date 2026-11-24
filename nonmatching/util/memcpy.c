/*
 * memcpy -- stock 0x40035724 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b __aeabi_memcpy.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "libc.h"
#include "stock_asm.h"
#include "util.h"
#include "shared/framebuffer.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void *__aeabi_memcpy_raw(void *d, const void *s, unsigned int n) __asm__("__aeabi_memcpy");

void *memcpy(void *d, const void *s, unsigned int n) { return __aeabi_memcpy_raw(d, s, n); }
