/*
 * settings_g_save -- stock 0x4004472C (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves record G: encode, then write 0x1000 bytes to the flash address SYS+0x7E8.
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
void settings_g_fields(void);

void settings_g_save(void)
{
    serializer_set_encode_mode();
    settings_g_fields();
    persistence_write_read_modify_write(0, SYS32(0x7E8), SER_IMAGE, 0x1000);
}
