/*
 * serializer_process_field -- stock 0x40067450 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Encodes or decodes one field, by the mode byte. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void serializer_encode_field(const volatile void *src, u32 size);
void serializer_decode_field(volatile void *dst, u32 size);

void serializer_process_field(volatile void *p, u32 size)
{
    if (SER_MODE == 0)
        serializer_encode_field(p, size);
    else
        serializer_decode_field(p, size);
}
