/* Functions of nonmatching/libc/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef LIBC_H
#define LIBC_H

#include "rt/bits.h"
#include "rt/ldiv.h"

AEABI void __ldivmod_c(s64 n, s64 d, u64qr *o);                                                  /* 0x4003453C */
AEABI void __uldivmod_c(u64 n, u64 d, u64qr *o);                                                 /* 0x40034590 */
AEABI u64 __aeabi_l2d(s64 x);                                                                    /* 0x4003A5D8 */
AEABI s64 __aeabi_d2lz(u32 lo, u32 hi);                                                          /* 0x4003DF94 */
u64 __aeabi_uidivmod(u32 n, u32 d);                                                              /* 0x4003E6F8 */
void __aeabi_ldiv0(void);                                                                        /* 0x4003E704 */
AEABI void __aeabi_memcpy(void *dst, const void *src, u32 n);                                    /* 0x4005C31C */
AEABI void __aeabi_memcpy4(void *dst, const void *src, u32 n);                                   /* 0x4005C340 */
AEABI void __memcpy_dst_unaligned(void *dst, const void *src, u32 n);                            /* 0x4005C38C */
AEABI u64 __aeabi_d2ulz(u32 lo, u32 hi);                                                         /* 0x4005F6E8 */
AEABI u64 __aeabi_ul2d(u64 x);                                                                   /* 0x4005F730 */
AEABI void __aeabi_memclr4(void *dest, u32 n);                                                   /* 0x40068E5C */
s64 llabs(s64 x);                                                                                /* 0x4006A4DA */
AEABI s64 __aeabi_f2lz(u32 x);                                                                   /* 0x4006A4E8 */
unsigned long long __aeabi_llsl(unsigned long long v, int sh);                                   /* 0x4006A530 */
AEABI void __aeabi_memset4(void *dest, u32 n, u32 c);                                            /* 0x4006A6FC */

#endif
