/*
 * gt911_write16 -- stock 0x400498C4 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: write len bytes at a 16-bit register address.
 */
#include "hw/drivers.h"
#include "drivers/gt911.h"

void gt911_write16(u32 reg, const unsigned char *buf, int len)
{
    gt911_write_regs(reg, 2, buf, len);
}
