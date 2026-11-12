/*
 * __USBC_ConfigFifo_TxEp -- stock 0x4006C9A8 (84 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FIFO size code = log2 of the 512-aligned size in 16-byte units; address in 8-byte units; bit 4 = double buffer.
 */
#include "hw/usb.h"

void __USBC_ConfigFifo_TxEp(u32 is_double_fifo, u32 fifo_size, u32 fifo_addr)
{
    u32 temp = ((fifo_size + 511) & ~511u) >> 4, size = 0;
    while (temp) {
        size++;
        temp >>= 1;
    }
    REG16(USBC_TXFIFOAD) = (unsigned short)(fifo_addr >> 3);
    REG8(USBC_TXFIFOSZ) = (unsigned char)(size & 0x0F);
    if (is_double_fifo)
        REG8(USBC_TXFIFOSZ) = (unsigned char)(REG8(USBC_TXFIFOSZ) | 0x10);
}
