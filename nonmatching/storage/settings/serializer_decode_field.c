/*
 * serializer_decode_field -- stock 0x40067420 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decodes size bytes at the cursor into dst, then advances the cursor. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void serializer_decode_scalar_be(u32 off, volatile void *dst, u32 size);

void serializer_decode_field(volatile void *dst, u32 size)
{
    serializer_decode_scalar_be(SER_CURSOR, dst, size);
    SER_CURSOR = SER_CURSOR + size;
}
