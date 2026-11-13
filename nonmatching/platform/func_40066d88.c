/*
 * console_print_num -- stock 0x40066D88 (328 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Integer to text: base < 0 = signed |base|; base 16 uses masks; base 10 inserts ',' every three digits; width < 0 = left aligned; width > 80 ignored; pad 0 means ' '.
 */
#include "hw/platform.h"

void console_print_num(int val, int base, int width, int pad)
{
    char buf[40];
    int len = 0, left = 0, neg = 0;
    u32 v = (u32)val, d;
    if (!pad) pad = ' ';
    if (width < 0) { width = -width; left = 1; }
    if ((u32)width >= 0x51) width = 0;
    if (base < 0) {
        base = -base;
        if (val < 0) { neg = 1; v = (u32)-val; }
    }
    for (;;) {
        if (base == 16) {
            d = v & 0xF;
            v >>= 4;
        } else {
            /* the stock UDIV yields 0 for base 0 (format "%" + NUL); C division by 0 is undefined */
            u32 q = base ? v / (u32)base : 0;
            d = v - q * (u32)base;
            v = q;
        }
        buf[len++] = (char)(d + ((unsigned short)d < 10 ? '0' : '7'));
        if (v == 0) break;
        if (base == 10 && (len == 3 || len == 7 || len == 11 || len == 15))
            buf[len++] = ',';
    }
    if (neg) buf[len++] = '-';
    if (len >= width) {
        console_put_reversed(buf, len);
        return;
    }
    if (!left) console_putn(pad, width - len);
    console_put_reversed(buf, len);
    if (left) console_putn(pad, width - len);
}
