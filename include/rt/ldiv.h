/* Shared core of the 64-bit AEABI division rewrites (stock 0x40034590 / 0x4003453C).
 * Reproduces the stock results including the zero-divisor behaviour:
 *   d == 0, n < 2^32 : UDIV by zero -> q = 0, r = n
 *   d == 0, n >= 2^32: __aeabi_ldiv0 returns with r0-r3 unchanged -> q = n, r = 0 */
#ifndef RT_LDIV_H
#define RT_LDIV_H
#include "rt/bits.h"

typedef struct { u64 q, r; } u64qr;

static void uldivmod_core(u64 n, u64 d, u64qr *o)
{
    if (d == 0) {
        if ((n >> 32) == 0) { o->q = 0; o->r = n; }
        else { o->q = n; o->r = 0; }
        return;
    }
    u64 q = 0, r = 0;
    for (int i = 63; i >= 0; i--) {             /* restoring binary long division */
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d) { r -= d; q |= 1ULL << i; }
    }
    o->q = q; o->r = r;
}

/* Return q in r0:r1 and r in r2:r3 (AEABI) from a core that fills a u64qr at [sp+8]. */
#define AEABI_QR_SHIM(name, core)                                   \
    __attribute__((naked)) void name(void)                          \
    {                                                               \
        __asm__ volatile(                                           \
            "push {r4, lr}\n\t"                                     \
            "sub sp, sp, #24\n\t"                                   \
            "add r4, sp, #8\n\t"                                    \
            "str r4, [sp]\n\t"                                      \
            "bl " #core "\n\t"                                      \
            "ldrd r0, r1, [sp, #8]\n\t"                             \
            "ldrd r2, r3, [sp, #16]\n\t"                            \
            "add sp, sp, #24\n\t"                                   \
            "pop {r4, pc}\n\t");                                    \
    }

#endif
