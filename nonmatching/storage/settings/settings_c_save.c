/*
 * settings_c_save -- stock 0x40043C68 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves settings record C (field list 0x40043B0C): encode the field list, then write 0x4000 bytes of the image to the flash address SYS_INFO+0x7F0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_c_save(void)
{
    serializer_set_encode_mode();
    record_c_fields();
    persistence_write_read_modify_write(0, SYS32(0x7F0), SER_IMAGE, 0x4000);
}
