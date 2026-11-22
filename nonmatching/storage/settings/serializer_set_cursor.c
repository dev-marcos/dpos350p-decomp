/*
 * serializer_set_cursor -- stock 0x40067128 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Store r0 at 0x4091F38C.
 */
#include "hw/platform.h"

void serializer_set_cursor(u32 v) { *(u32 *)0x4091F38Cu = v; }
