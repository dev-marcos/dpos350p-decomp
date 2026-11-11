/* Bit-level helpers for the run-time library rewrites (nonmatching/libc, nonmatching/libm).
 * ARM register shifts saturate (shift >= 32 gives 0); C shifts do not, so every shift whose
 * amount can reach 32 goes through these helpers. */
#ifndef RT_BITS_H
#define RT_BITS_H

typedef unsigned int       u32;
typedef signed int         s32;
typedef unsigned long long u64;
typedef signed long long   s64;

static inline u32 lsr32(u32 x, u32 n) { return n >= 32 ? 0 : x >> n; }
static inline u32 lsl32(u32 x, u32 n) { return n >= 32 ? 0 : x << n; }
static inline u32 clz32(u32 x) { return x ? (u32)__builtin_clz(x) : 32; }

static inline u64 dbits(double d) { union { double d; u64 u; } v; v.d = d; return v.u; }
static inline double bitsd(u64 u) { union { double d; u64 u; } v; v.u = u; return v.d; }
static inline u32 fbits(float f) { union { float f; u32 u; } v; v.f = f; return v.u; }

/* AEABI helpers use the base (soft-float) procedure call standard */
#define AEABI __attribute__((pcs("aapcs")))

#endif
