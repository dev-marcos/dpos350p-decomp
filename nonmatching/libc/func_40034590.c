/*
 * __aeabi_uldivmod -- stock 0x40034590 (304 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: 64-bit unsigned divide; quotient in r0:r1, remainder in r2:r3.
 */
#include "rt/ldiv.h"

AEABI void __uldivmod_c(u64 n, u64 d, u64qr *o) { uldivmod_core(n, d, o); }
AEABI_QR_SHIM(__aeabi_uldivmod, __uldivmod_c)
