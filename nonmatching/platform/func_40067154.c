/*
 * serializer_get_cursor -- stock 0x40067154 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Returns the serialiser cursor 0x4091F38C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */

u32 serializer_get_cursor(void)
{
    return SER_CURSOR;
}
