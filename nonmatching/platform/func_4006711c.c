/*
 * debug_printf_stub -- stock 0x4006711C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Variadic no-op (the usbprint/debug macro target).
 */
#include "hw/platform.h"

void debug_printf_stub(const char *fmt, ...) { (void)fmt; }
