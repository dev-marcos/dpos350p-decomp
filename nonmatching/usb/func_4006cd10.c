/*
 * USBC_WritePacket -- stock 0x4006CD10 (68 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy cnt bytes to a FIFO: 32-bit words, then the tail bytes. Returns cnt.
 */
#include "hw/usb.h"

u32 USBC_WritePacket(void *fifo, u32 cnt, void *buff)
{
    u32 i, *b32 = buff;
    unsigned char *b8;
    for (i = cnt >> 2; i; i--)
        REG32(fifo) = *b32++;
    b8 = (unsigned char *)b32;
    for (i = cnt & 3; i; i--)
        REG8(fifo) = *b8++;
    return cnt;
}
