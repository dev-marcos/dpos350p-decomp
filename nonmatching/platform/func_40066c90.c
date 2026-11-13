/*
 * uart_putc -- stock 0x40066C90 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Wait for TX FIFO not full (USR bit 1), write THR. Port 0/1/2 = 0x01C28000/400/800 (another port uses an unset register: OPEN, not tested).
 */
#include "hw/platform.h"

void uart_putc(int port, int c)
{
    u32 base = 0;
    if (port == 0) base = 0x01C28000u;
    else if (port == 2) base = 0x01C28800u;
    else if ((u32)port < 2) base = 0x01C28400u;
    while (!(readl_g(base + 0x7C) & 2))
        ;
    writel_g(base, (u32)c);
}
