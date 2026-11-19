/*
 * settings_c_load_tail -- stock 0x40043CA8 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Second half of settings_c_load (the 12-byte entry falls through into it with p = SYS_INFO, off = 0x360404, len = 0x4000): read len bytes from the flash address p+0x7F0 into p + off, then decode the field list 0x40043B0C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void persistence_read(u32 dev, u32 addr, volatile void *buf, u32 len);
void serializer_set_decode_mode(void);
void record_c_fields(void);

void settings_c_load_tail(volatile unsigned char *p, u32 off, u32 unused, u32 len)
{
    (void)unused;
    persistence_read(0, *(volatile u32 *)(p + 0x7F0), p + off, len);
    serializer_set_decode_mode();
    record_c_fields();
}
