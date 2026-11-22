/*
 * settings_d_load -- stock 0x40043D88 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads settings record D: read 0x2000 bytes from the flash address SYS_INFO+0x7EC into the image, then decode the field list.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
void serializer_process_field(volatile void *p, u32 size);
void persistence_read(u32 dev, u32 addr, volatile void *buf, u32 len);
void persistence_write_read_modify_write(u32 dev, u32 addr, volatile void *buf, u32 len);
#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_d_fields(void);

void settings_d_load(void)
{
    persistence_read(0, SYS32(0x7EC), SER_IMAGE, 0x2000);
    serializer_set_decode_mode();
    settings_d_fields();
}
