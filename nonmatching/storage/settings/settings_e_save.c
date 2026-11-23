/*
 * settings_e_save -- stock 0x40043E00 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves settings record E (four words at 0x42E68AE0): encode the field list, then write 0x1000 bytes of the image to the flash address SYS_INFO+0x7DC.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_e_save(void)
{
    serializer_set_encode_mode();
    settings_e_fields();
    persistence_write_read_modify_write(0, SI(rec_e_flash_addr), SER_IMAGE, 0x1000);
}
