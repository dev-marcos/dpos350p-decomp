/*
 * gt911_read16 -- stock 0x400498D4 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: read len bytes from a 16-bit register address.
 */
#include "hw/drivers.h"

void gt911_read16(u32 reg, unsigned char *buf, int len)
{
    gt911_read_regs(reg, 2, buf, len);
}
