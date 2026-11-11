/*
 * gt911_write_regs -- stock 0x40049688 (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: write len bytes to a register (device address 0xBA; 8- or 16-bit register address).
 */
#include "hw/drivers.h"

void gt911_write_regs(u32 reg, u32 addr_len, const unsigned char *buf, int len)
{
    int i;
    gt911_i2c_start();
    gt911_write_byte(0xBA);
    gt911_wait_ack();
    if (addr_len == 1) goto lo;
    if (addr_len == 2) {
        gt911_write_byte(reg >> 8);
        gt911_wait_ack();
lo:
        gt911_write_byte(reg & 0xFF);
        gt911_wait_ack();
    }
    for (i = 0; i < len; i++) {
        gt911_write_byte(buf[i]);
        gt911_wait_ack();
    }
    gt911_i2c_stop();
}
