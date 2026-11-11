/*
 * __aeabi_ldivmod -- stock 0x4003453C (84 bytes, ARM)
 * Status: NONMATCHING; behaviour verified by tools/emu (docs/PROGRESS.md)
 * Arm C library: 64-bit signed divide on top of __aeabi_uldivmod. The quotient is negated
 * when the operand signs differ; the remainder takes the sign of the dividend.
 */
#include "rt/ldiv.h"

AEABI void __ldivmod_c(s64 n, s64 d, u64qr *o)
{
    int sn = n < 0, sd = d < 0;
    uldivmod_core(sn ? 0 - (u64)n : (u64)n, sd ? 0 - (u64)d : (u64)d, o);
    if (sn ^ sd) o->q = 0 - o->q;
    if (sn) o->r = 0 - o->r;
}
AEABI_QR_SHIM(__aeabi_ldivmod, __ldivmod_c)
