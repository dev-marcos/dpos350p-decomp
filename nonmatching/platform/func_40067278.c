/*
 * serializer_encode_field -- stock 0x40067278 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Encodes size bytes from src at the cursor, then advances the cursor. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void serializer_encode_scalar_be(u32 off, const volatile void *src, u32 size);

void serializer_encode_field(const volatile void *src, u32 size)
{
    serializer_encode_scalar_be(SER_CURSOR, src, size);
    SER_CURSOR = SER_CURSOR + size;
}
