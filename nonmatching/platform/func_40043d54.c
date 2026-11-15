/*
 * settings_d_save -- stock 0x40043D54 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves settings record D (0x42E554B0, 300 entries): encode the field list, then write 0x2000 bytes of the image to the flash address SYS_INFO+0x7EC.
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

void settings_d_save(void)
{
    serializer_set_encode_mode();
    settings_d_fields();
    persistence_write_read_modify_write(0, SYS32(0x7EC), SER_IMAGE, 0x2000);
}
