/* Volatile access macros used by many C files: B8 / SB8 (byte), H16 (halfword), W32 (word),
 * S64 (signed 64-bit), F32 (float), D64 (double) at an absolute address (build 2025-06-19). */
#ifndef SHARED_ACCESS_H
#define SHARED_ACCESS_H

#define B8(a) (*(volatile unsigned char *)(a))
#define D64(a) (*(volatile double *)(a))
#define F32(a) (*(volatile float *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define S64(a) (*(volatile long long *)(a))
#define SB8(a) (*(volatile signed char *)(a))
#define W32(a) (*(volatile u32 *)(a))

#endif /* SHARED_ACCESS_H */
