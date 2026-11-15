/*
 * settings_e_save -- stock 0x40043E00 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves settings record E (four words at 0x42E68AE0): encode the field list, then write 0x1000 bytes of the image to the flash address SYS_INFO+0x7DC.
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

void settings_e_fields(void);

void settings_e_save(void)
{
    serializer_set_encode_mode();
    settings_e_fields();
    persistence_write_read_modify_write(0, SYS32(0x7DC), SER_IMAGE, 0x1000);
}
