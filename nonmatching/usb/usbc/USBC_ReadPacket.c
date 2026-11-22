/*
 * USBC_ReadPacket -- stock 0x4006CD54 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy cnt bytes from a FIFO: 32-bit words, then the tail bytes. Returns cnt.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

u32 USBC_ReadPacket(void *fifo, u32 cnt, void *buff)
{
    u32 i, *b32 = buff;
    unsigned char *b8;
    for (i = cnt >> 2; i; i--)
        *b32++ = REG32(fifo);
    b8 = (unsigned char *)b32;
    for (i = cnt & 3; i; i--)
        *b8++ = REG8(fifo);
    return cnt;
}
