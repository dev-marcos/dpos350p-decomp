/*
 * gt911_read_regs -- stock 0x40049700 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * GT911: set the register address (write 0xBA), repeated start, read len bytes (0xBB), ACK after every byte.
 */
#include "hw/drivers.h"

void gt911_read_regs(u32 reg, u32 addr_len, unsigned char *buf, int len)
{
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
    gt911_i2c_start();
    gt911_write_byte(0xBB);
    gt911_wait_ack();
    while (len) {
        *buf++ = gt911_read_byte();
        gt911_send_ack();
        len--;
    }
    gt911_i2c_stop();
}
